#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AsteroidLevelManager.generated.h"

class AAsteroid;

UCLASS()
class YAAC_API AAsteroidLevelManager : public AActor
{
	GENERATED_BODY()
    
public:    
	AAsteroidLevelManager();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// Asteroid Blueprint class to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AAsteroid> AsteroidClass;

	// Base number of asteroids in Wave 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	int32 BaseAsteroidCount = 4;

	// Additional asteroids added per level cycle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	int32 AsteroidsPerLevelIncrease = 2;

	// Playfield bounds matching AShip2 / AShipProjectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	FVector PlayfieldBounds = FVector(600.0f, 1050.0f, 92.0f);

	// Trackers
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	int32 CurrentWave = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float WaveDelay = 2.0f; // Delay before next wave starts

	bool bWaveInProgress = false;

	// Logic
	void StartNextWave();
	void SpawnAsteroids(int32 CountToSpawn);
	FVector GetRandomScreenEdgeLocation();

	FTimerHandle WaveTimerHandle;
};