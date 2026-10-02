#include "Asteroid.h"

#include "NiagaraFunctionLibrary.h"
#include "../Ship/Bullet/Bullet.h"
#include "../Gamemode/AsteroidGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AAsteroid::AAsteroid()
{
    PrimaryActorTick.bCanEverTick = true;

    AsteroidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AsteroidMesh"));
    RootComponent = AsteroidMesh;

    // Set up collision profile
    AsteroidMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    AsteroidMesh->SetGenerateOverlapEvents(true);
}

void AAsteroid::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap event for bullet hits
    AsteroidMesh->OnComponentBeginOverlap.AddDynamic(this, &AAsteroid::OnOverlapBegin);

    // If spawned directly without custom initialization, assign default random movement
    if (MovementDirection.IsZero())
    {
        FVector RandomDir = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f);
        InitializeAsteroid(AsteroidSize, RandomDir);
    }
}

void AAsteroid::InitializeAsteroid(EAsteroidSize NewSize, FVector Direction)
{
    AsteroidSize = NewSize;
    MovementDirection = Direction.GetSafeNormal();

    // Scale mesh, speed, and point values based on size
    float ScaleMultiplier = 1.0f;
    switch (AsteroidSize)
    {
    case EAsteroidSize::Large:
        ScaleMultiplier = 2.0f;
        MoveSpeed = FMath::RandRange(100.0f, 200.0f);
        break;
    case EAsteroidSize::Medium:
        ScaleMultiplier = 1.0f;
        MoveSpeed = FMath::RandRange(200.0f, 350.0f);
        break;
    case EAsteroidSize::Small:
        ScaleMultiplier = 0.5f;
        MoveSpeed = FMath::RandRange(350.0f, 500.0f);
        break;
    }

    SetActorScale3D(FVector(ScaleMultiplier));

    // Random tumble rotation speed
    RotationRate = FRotator(
        0.0f,
        FMath::RandRange(-100.0f, 100.0f), // Yaw rotation
        0.0f
    );
}

void AAsteroid::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 1. Drift movement
    AddActorWorldOffset(MovementDirection * MoveSpeed * DeltaTime, true);

    // 2. Tumble rotation
    AddActorLocalRotation(RotationRate * DeltaTime);

    // 3. Screen Wrapping Logic
    FVector bounds = FVector(600.0f, 1050.0f, 0.0f);
    FVector location = GetActorLocation();

    if (location.X > bounds.X)  location.X -= 2 * bounds.X;
    if (location.X < -bounds.X) location.X += 2 * bounds.X;
    if (location.Y > bounds.Y)  location.Y -= 2 * bounds.Y;
    if (location.Y < -bounds.Y) location.Y += 2 * bounds.Y;

    SetActorLocation(location);
}

void AAsteroid::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                               bool bFromSweep, const FHitResult& SweepResult)
{
    // If struck by a bullet
    if (OtherActor && OtherActor->IsA(ABullet::StaticClass()))
    {
        OtherActor->Destroy(); // Destroy bullet
        BreakApart();          // Split or destroy asteroid
    }
}

void AAsteroid::PlayDeathEffects()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    float ScaleMultiplier = 1.0f;
    switch (AsteroidSize)
    {
    case EAsteroidSize::Large:
        ScaleMultiplier = 2.0f;
        break;
    case EAsteroidSize::Medium:
        ScaleMultiplier = 1.0f;
        break;
    case EAsteroidSize::Small:
        ScaleMultiplier = 0.5f;
        break;
    }

    // 1. Spawn and configure Niagara Particle System
    if (ExplosionEffect)
    {
        UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            ExplosionEffect,
            GetActorLocation(),
            GetActorRotation(),
            FVector(0.8*ScaleMultiplier), // Makes the effect smaller
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

void AAsteroid::BreakApart()
{
    UWorld* World = GetWorld();
    if (World)
    {
        PlayDeathEffects();
        // 1. Award Points based on Size
        AAsteroidGameMode* GM = Cast<AAsteroidGameMode>(World->GetAuthGameMode());
        if (GM)
        {
            int32 PointsToGive = 20; // Default Small
            if (AsteroidSize == EAsteroidSize::Large)  PointsToGive = 100;
            if (AsteroidSize == EAsteroidSize::Medium) PointsToGive = 50;
            
            GM->AddScore(PointsToGive);
        }

        // 2. Split logic (existing code)
        EAsteroidSize NextSize;
        bool bShouldSplit = false;

        if (AsteroidSize == EAsteroidSize::Large)
        {
            NextSize = EAsteroidSize::Medium;
            bShouldSplit = true;
        }
        else if (AsteroidSize == EAsteroidSize::Medium)
        {
            NextSize = EAsteroidSize::Small;
            bShouldSplit = true;
        }

        if (bShouldSplit)
        {
            for (int32 i = 0; i < 2; i++)
            {
                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                AAsteroid* NewAsteroid = World->SpawnActor<AAsteroid>(GetClass(), GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
                if (NewAsteroid)
                {
                    FVector RandomDir = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f);
                    NewAsteroid->InitializeAsteroid(NextSize, RandomDir);
                }
            }
        }
    }

    Destroy();
}