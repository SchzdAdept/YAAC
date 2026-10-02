#include "VisualWrapComponent.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"

UVisualWrapComponent::UVisualWrapComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UVisualWrapComponent::BeginPlay()
{
    Super::BeginPlay();
    MaxX = PlayAreaWidth / 2.0f;
    MinX = -MaxX;
    MaxY = PlayAreaHeight / 2.0f;
    MinY = -MaxY;
}

void UVisualWrapComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateGhosts();
}

void UVisualWrapComponent::UpdateGhosts()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector Loc = Owner->GetActorLocation();
    FRotator Rot = Owner->GetActorRotation();

    bool bNearLeft   = (Loc.X < MinX + DetectionBuffer);
    bool bNearRight  = (Loc.X > MaxX - DetectionBuffer);
    bool bNearBottom = (Loc.Y < MinY + DetectionBuffer);
    bool bNearTop    = (Loc.Y > MaxY - DetectionBuffer);

    // Lambda helper to position the ghost AND sync its dynamic scale configurations
    auto UpdateGhostTransformAndScale = [this, Owner, Rot](AActor* Ghost, float OffsetX, float OffsetY)
    {
        if (!Ghost) return;
        
        // 1. Match Location and Rotation
        FVector CurrentLoc = Owner->GetActorLocation();
        Ghost->SetActorLocationAndRotation(FVector(CurrentLoc.X + OffsetX, CurrentLoc.Y + OffsetY, CurrentLoc.Z), Rot);
        
        // 2. Mirror Master Actor Scale
        Ghost->SetActorScale3D(Owner->GetActorScale3D());

        // 3. Dynamic Sub-Component Scale Sync (For meshes/particles resizing mid-flight)
        UStaticMeshComponent* OwnerMesh = Owner->FindComponentByClass<UStaticMeshComponent>();
        UStaticMeshComponent* GhostMesh = Ghost->FindComponentByClass<UStaticMeshComponent>();
        if (OwnerMesh && GhostMesh)
        {
            GhostMesh->SetRelativeScale3D(OwnerMesh->GetRelativeScale3D());
        }

        TArray<UNiagaraComponent*> OwnerNiagaras;
        Owner->GetComponents<UNiagaraComponent>(OwnerNiagaras);
        
        TArray<UNiagaraComponent*> GhostNiagaras;
        Ghost->GetComponents<UNiagaraComponent>(GhostNiagaras);

        // Map and update each niagara emitter component by index
        for (int32 i = 0; i < OwnerNiagaras.Num(); ++i)
        {
            if (GhostNiagaras.IsValidIndex(i) && OwnerNiagaras[i])
            {
                GhostNiagaras[i]->SetRelativeScale3D(OwnerNiagaras[i]->GetRelativeScale3D());
            }
        }
    };

    // --- 1. X-AXIS GHOST ---
    if (bNearLeft || bNearRight)
    {
        if (!GhostX) GhostX = SpawnGhostClone();
        float OffsetX = bNearLeft ? PlayAreaWidth : -PlayAreaWidth;
        UpdateGhostTransformAndScale(GhostX, OffsetX, 0.0f);
    }
    else { DestroyGhost(GhostX); }

    // --- 2. Y-AXIS GHOST ---
    if (bNearBottom || bNearTop)
    {
        if (!GhostY) GhostY = SpawnGhostClone();
        float OffsetY = bNearBottom ? PlayAreaHeight : -PlayAreaHeight;
        UpdateGhostTransformAndScale(GhostY, 0.0f, OffsetY);
    }
    else { DestroyGhost(GhostY); }

    // --- 3. CORNER GHOST ---
    if ((bNearLeft || bNearRight) && (bNearBottom || bNearTop))
    {
        if (!GhostCorner) GhostCorner = SpawnGhostClone();
        float OffsetX = bNearLeft ? PlayAreaWidth : -PlayAreaWidth;
        float OffsetY = bNearBottom ? PlayAreaHeight : -PlayAreaHeight;
        UpdateGhostTransformAndScale(GhostCorner, OffsetX, OffsetY);
    }
    else { DestroyGhost(GhostCorner); }
}


AActor* UVisualWrapComponent::SpawnGhostClone()
{
    AActor* Owner = GetOwner();
    if (!Owner) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    // 1. Spawn the empty actor container
    AActor* Ghost = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Owner->GetActorLocation(), Owner->GetActorRotation(), SpawnParams);
    
    if (Ghost)
    {
        // IMMEDIATELY match the actor scale so the root starts correct
        Ghost->SetActorScale3D(Owner->GetActorScale3D());

        USceneComponent* RootComp = nullptr;
        UStaticMeshComponent* OwnerMesh = Owner->FindComponentByClass<UStaticMeshComponent>();

        // 2. CLONE STATIC MESH
        if (OwnerMesh)
        {
            UStaticMeshComponent* GhostMesh = NewObject<UStaticMeshComponent>(Ghost, UStaticMeshComponent::StaticClass());
            
            // CRITICAL: Set all scale, location, and rotation BEFORE registering the component
            GhostMesh->SetStaticMesh(OwnerMesh->GetStaticMesh());
            GhostMesh->SetRelativeLocation(OwnerMesh->GetRelativeLocation());
            GhostMesh->SetRelativeRotation(OwnerMesh->GetRelativeRotation());
            GhostMesh->SetRelativeScale3D(OwnerMesh->GetRelativeScale3D());

            for (int32 i = 0; i < OwnerMesh->GetNumMaterials(); ++i)
            {
                GhostMesh->SetMaterial(i, OwnerMesh->GetMaterial(i));
            }

            GhostMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            GhostMesh->SetGenerateOverlapEvents(false);
            
            // Set it as root and register
            Ghost->SetRootComponent(GhostMesh);
            GhostMesh->RegisterComponent();
            
            RootComp = GhostMesh;
        }
        else
        {
            RootComp = NewObject<USceneComponent>(Ghost, USceneComponent::StaticClass());
            Ghost->SetRootComponent(RootComp);
            RootComp->RegisterComponent();
        }

        // 3. CLONE NIAGARA COMPONENTS
        TArray<UNiagaraComponent*> OwnerNiagaraComps;
        Owner->GetComponents<UNiagaraComponent>(OwnerNiagaraComps);

        for (UNiagaraComponent* OwnerNiagara : OwnerNiagaraComps)
        {
            if (OwnerNiagara && OwnerNiagara->GetAsset())
            {
                UNiagaraComponent* GhostNiagara = NewObject<UNiagaraComponent>(Ghost, UNiagaraComponent::StaticClass());
                
                // Set the asset and properties BEFORE registering or activating
                GhostNiagara->SetAsset(OwnerNiagara->GetAsset());
                GhostNiagara->SetRelativeLocation(OwnerNiagara->GetRelativeLocation());
                GhostNiagara->SetRelativeRotation(OwnerNiagara->GetRelativeRotation());
                GhostNiagara->SetRelativeScale3D(OwnerNiagara->GetRelativeScale3D());

                // Attach to the root component cleanly using relative transforms
                GhostNiagara->SetupAttachment(RootComp);
                
                // Now register it to the world render pipeline
                GhostNiagara->RegisterComponent();
                GhostNiagara->Activate(true);
            }
        }
    }
    return Ghost;
}

void UVisualWrapComponent::DestroyGhost(AActor*& GhostActor)
{
    if (GhostActor)
    {
        GhostActor->Destroy();
        GhostActor = nullptr;
    }
}

void UVisualWrapComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DestroyGhost(GhostX);
    DestroyGhost(GhostY);
    DestroyGhost(GhostCorner);
    Super::EndPlay(EndPlayReason);
}