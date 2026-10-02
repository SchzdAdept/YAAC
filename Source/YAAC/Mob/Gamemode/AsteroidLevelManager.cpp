#include "AsteroidLevelManager.h"
#include "AsteroidGameMode.h"
#include "../Asteroid/Asteroid.h"
#include "Kismet/GameplayStatics.h"

class AAsteroidGameMode;

AAsteroidLevelManager::AAsteroidLevelManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AAsteroidLevelManager::BeginPlay()
{
    Super::BeginPlay();

    // Register this LevelManager with the GameMode
    AAsteroidGameMode* GM = Cast<AAsteroidGameMode>(GetWorld()->GetAuthGameMode());
    if (GM)
    {
        GM->RegisterLevelManager(this);
    }

    // Start wave 1 shortly after level loads
    GetWorldTimerManager().SetTimer(WaveTimerHandle, this, &AAsteroidLevelManager::StartNextWave, 1.0f, false);
}

void AAsteroidLevelManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bWaveInProgress) return;

    // Find all active asteroids remaining
    TArray<AActor*> FoundAsteroids;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAsteroid::StaticClass(), FoundAsteroids);

    // When all asteroids are destroyed
    if (FoundAsteroids.Num() == 0)
    {
        bWaveInProgress = false;

        // Notify GameMode to reset ship position to center
        AAsteroidGameMode* GM = Cast<AAsteroidGameMode>(GetWorld()->GetAuthGameMode());
        if (GM)
        {
            GM->OnWaveCompleted();
        }

        // Schedule next wave start
        GetWorldTimerManager().SetTimer(WaveTimerHandle, this, &AAsteroidLevelManager::StartNextWave, WaveDelay, false);
    }
}

void AAsteroidLevelManager::StartNextWave()
{
    CurrentWave++;

    // Calculate how many large asteroids to spawn this cycle
    int32 AsteroidsToSpawn = BaseAsteroidCount + ((CurrentWave - 1) * AsteroidsPerLevelIncrease);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(6, 3.0f, FColor::Green, FString::Printf(TEXT("STARTING WAVE %d (%d Asteroids)"), CurrentWave, AsteroidsToSpawn));
    }

    SpawnAsteroids(AsteroidsToSpawn);
    bWaveInProgress = true;
}

void AAsteroidLevelManager::SpawnAsteroids(int32 CountToSpawn)
{
    if (!AsteroidClass) return;

    UWorld* World = GetWorld();
    if (!World) return;

    for (int32 i = 0; i < CountToSpawn; i++)
    {
        FVector SpawnLocation = GetRandomScreenEdgeLocation();
        FRotator SpawnRotation = FRotator::ZeroRotator;

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AAsteroid* NewAsteroid = World->SpawnActor<AAsteroid>(AsteroidClass, SpawnLocation, SpawnRotation, SpawnParams);
        if (NewAsteroid)
        {
            // Pick a random drift direction pushing inward towards center
            FVector RandomDir = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f);
            NewAsteroid->InitializeAsteroid(EAsteroidSize::Large, RandomDir);
        }
    }
}

// Generates spawn locations strictly along the outer edges of the screen boundary
FVector AAsteroidLevelManager::GetRandomScreenEdgeLocation()
{
    float EdgePadding = 50.0f; // Slightly outside visible area
    FVector Location = FVector::ZeroVector;

    // Pick 1 of 4 edges randomly (0: Top, 1: Bottom, 2: Left, 3: Right)
    int32 Edge = FMath::RandRange(0, 3);

    switch (Edge)
    {
    case 0: // Top Edge
        Location.X = PlayfieldBounds.X + EdgePadding;
        Location.Y = FMath::RandRange(-PlayfieldBounds.Y, PlayfieldBounds.Y);
        break;
    case 1: // Bottom Edge
        Location.X = -PlayfieldBounds.X - EdgePadding;
        Location.Y = FMath::RandRange(-PlayfieldBounds.Y, PlayfieldBounds.Y);
        break;
    case 2: // Right Edge
        Location.X = FMath::RandRange(-PlayfieldBounds.X, PlayfieldBounds.X);
        Location.Y = PlayfieldBounds.Y + EdgePadding;
        break;
    case 3: // Left Edge
        Location.X = FMath::RandRange(-PlayfieldBounds.X, PlayfieldBounds.X);
        Location.Y = -PlayfieldBounds.Y - EdgePadding;
        break;
    }
    
    Location.Z = 92.0f;
    return Location;
}