#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AsteroidGameMode.generated.h"

class AAsteroidLevelManager;
class AShip2;

UCLASS()
class YAAC_API AAsteroidGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAsteroidGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules")
	int32 PlayerLives = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Rules")
	int32 CurrentScore = 0;

	// Pointer to active level manager
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Rules")
	AAsteroidLevelManager* LevelManager;

	// Called by LevelManager on BeginPlay
	void RegisterLevelManager(AAsteroidLevelManager* Manager);

	// Called when a wave is completed
	void OnWaveCompleted();

	// Resets ship position to center and gives invulnerability
	void ResetShipForNewWave();

	void AddScore(int32 Points);
	void OnPlayerDied();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Game Rules")
	void OnGameEnded();

protected:
	UPROPERTY(EditAnywhere, Category = "Game Rules")
	float RespawnDelay = 2.0f;

	void RespawnPlayer();

	FTimerHandle RespawnTimerHandle;
};