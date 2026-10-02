#include "Ship2.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Bullet/Bullet.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Mob/Asteroid/Asteroid.h"
#include "Mob/Gamemode/AsteroidGameMode.h"

AShip2::AShip2()
{
    PrimaryActorTick.bCanEverTick = true;

    ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
    RootComponent = ShipMesh;

    ShipMesh->SetSimulatePhysics(true);
    ShipMesh->SetEnableGravity(false);
    ShipMesh->SetConstraintMode(EDOFMode::XYPlane);
    ShipMesh->SetLinearDamping(0.1f);
    ShipMesh->SetAngularDamping(2.0f);
    
    
    MuzzleLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("MuzzleLocation"));
    MuzzleLocation->SetupAttachment(RootComponent);
    MuzzleLocation->SetRelativeLocation(FVector(100.0f, 0.0f, 0.0f));
}

void AShip2::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    
    if (APlayerController* PlayerController = Cast<APlayerController>(NewController))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            if (AsteroidMappingContext)
            {
                Subsystem->AddMappingContext(AsteroidMappingContext, 0);
            }
        }
    }
}

void AShip2::EnhancedFire()
{
    if (!ProjectileClass) return;

    UWorld* const World = GetWorld();
    if (World)
    {
        // Get spawn transform from MuzzleLocation component
        FVector SpawnLocation = MuzzleLocation ? MuzzleLocation->GetComponentLocation() : GetActorLocation();
        FRotator SpawnRotation = GetActorRotation();

        FActorSpawnParameters ActorSpawnParams;
        ActorSpawnParams.Owner = this;
        ActorSpawnParams.Instigator = GetInstigator();
        ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // Spawn projectile into the world
        World->SpawnActor<ABullet>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
    }
}

void AShip2::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap for player-asteroid collision
    if (ShipMesh)
    {
        ShipMesh->OnComponentBeginOverlap.AddDynamic(this, &AShip2::OnShipOverlap);
    }

    // Input Mapping Context logic...
    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            if (AsteroidMappingContext)
            {
                Subsystem->AddMappingContext(AsteroidMappingContext, 0);
            }
        }
    }
    
    StartInvulnerability();
}


void AShip2::PlayDeathEffects()
{
    UWorld* World = GetWorld();
    if (!World) return;

    FVector DeathLocation = GetActorLocation();

    // 1. Spawn and configure Niagara Particle System
    if (ExplosionEffect)
    {
        UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            ExplosionEffect,
            DeathLocation,
            GetActorRotation(),
            FVector(0.7f), // Makes the effect smaller
            true,           // Auto Destroy when finished
            true,           // Auto Activate
            ENCPoolMethod::None,
            true            // Pre-Cull Check
        );

        if (NiagaraComp)
        {
            // Speeds up the playback of the particle simulation
            NiagaraComp->SetCustomTimeDilation(1.5);
        }
    }
}

void AShip2::OnShipOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                           bool bFromSweep, const FHitResult& SweepResult)
{
    // Ignore asteroid collisions while invulnerable
    if (bIsInvulnerable) return;

    if (OtherActor && OtherActor->IsA(AAsteroid::StaticClass()))
    {
        
        
        
        PlayDeathEffects();
        
        
        AAsteroidGameMode* GM = Cast<AAsteroidGameMode>(GetWorld()->GetAuthGameMode());
        if (GM)
        {
            GM->OnPlayerDied();
        }

        // Hide ship & disable collision during death delay
        SetActorEnableCollision(false);
        SetActorHiddenInGame(true);
        SetActorTickEnabled(false);

        // Clear timers if destroyed early
        GetWorldTimerManager().ClearTimer(BlinkTimerHandle);
        GetWorldTimerManager().ClearTimer(InvulnerabilityTimerHandle);

        SetLifeSpan(0.1f);
    }
}

void AShip2::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Cast to Enhanced Input Component
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Bind Thrust Action (Triggered handles continuous holding, Completed handles letting go)
        EnhancedInputComponent->BindAction(ThrustAction, ETriggerEvent::Triggered, this, &AShip2::EnhancedThrust);
        EnhancedInputComponent->BindAction(ThrustAction, ETriggerEvent::Completed, this, &AShip2::EnhancedThrust);

        // Bind Turn Action
        EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &AShip2::EnhancedTurn);
        EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Completed, this, &AShip2::EnhancedTurn);
        
        if (FireAction)
        {
            EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShip2::EnhancedFire);
        }
    }
    // Bind Fire Action (Started = fires once per press)

}

void AShip2::EnhancedThrust(const FInputActionValue& Value)
{
    // Extract float value from input system (returns 0.0f when action is completed/released)
    CurrentThrust = Value.Get<float>();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Green, FString::Printf(TEXT("Thrust Raw Value: %f"), CurrentThrust));
    }
}

void AShip2::EnhancedTurn(const FInputActionValue& Value)
{
    CurrentTurn = Value.Get<float>();
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Green, FString::Printf(TEXT("Turn Raw Value: %f"), CurrentTurn));
    }
}

void AShip2::StartInvulnerability()
{
    bIsInvulnerable = true;

    UWorld* World = GetWorld();
    if (World)
    {
        // 1. Loop blinking effect every BlinkInterval seconds
        World->GetTimerManager().SetTimer(BlinkTimerHandle, this, &AShip2::ToggleBlinkVisibility, BlinkInterval, true);

        // 2. Set timer to end invulnerability after InvulnerabilityDuration seconds
        World->GetTimerManager().SetTimer(InvulnerabilityTimerHandle, this, &AShip2::EndInvulnerability, InvulnerabilityDuration, false);
    }
}

void AShip2::ToggleBlinkVisibility()
{
    if (ShipMesh)
    {
        // Toggle mesh visibility between hidden and visible
        bool bIsVisible = ShipMesh->IsVisible();
        ShipMesh->SetVisibility(!bIsVisible, true);
    }
}

void AShip2::EndInvulnerability()
{
    bIsInvulnerable = false;

    UWorld* World = GetWorld();
    if (World)
    {
        // Clear timers
        World->GetTimerManager().ClearTimer(BlinkTimerHandle);
        World->GetTimerManager().ClearTimer(InvulnerabilityTimerHandle);
    }

    // Force mesh back to visible when invulnerability finishes
    if (ShipMesh)
    {
        ShipMesh->SetVisibility(true, true);
    }
}

void AShip2::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!ShipMesh) return;

    // 1. Screen Wrapping Logic
    FVector bounds = FVector(600.0f, 1050.0f, 0.0f);
    FVector location = GetActorLocation();

    if (location.X > bounds.X)  SetActorLocation(FVector(location.X - 2 * bounds.X, location.Y, location.Z));
    if (location.X < -bounds.X) SetActorLocation(FVector(location.X + 2 * bounds.X, location.Y, location.Z));
    if (location.Y > bounds.Y)  SetActorLocation(FVector(location.X, location.Y - 2 * bounds.Y, location.Z));
    if (location.Y < -bounds.Y) SetActorLocation(FVector(location.X, location.Y + 2 * bounds.Y, location.Z));

    // 2. Direct Transform Rotation (Yaw around Z-axis)
    if (FMath::Abs(CurrentTurn) > 0.0f)
    {
        // Calculate degrees to rotate this frame
        float DegreesToRotate = CurrentTurn * TurnRate * DeltaTime;
        
        // Apply rotation directly to the Actor around its local Z (Yaw) axis
        AddActorLocalRotation(FRotator(0.0f, DegreesToRotate, 0.0f));

        // Alternative: Zero out physics angular velocity so physics doesn't interfere
        ShipMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }

    // 3. Handle Forward Thrust (Pushes physics mesh in the direction the actor is facing)
    if (FMath::Abs(CurrentThrust) > 0.0f)
    {
        FVector ForwardDir = GetActorForwardVector();
        FVector Force = ForwardDir * CurrentThrust * ThrustAcceleration;
        ShipMesh->AddForce(Force, NAME_None, true); // true = bAccelChange (bypasses mass)
    }
}