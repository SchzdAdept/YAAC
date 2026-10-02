#include "AsteroidGameMode.h"
#include "AsteroidLevelManager.h"
#include "../Ship/Ship2.h"
#include "Kismet/GameplayStatics.h"

AAsteroidGameMode::AAsteroidGameMode()
{
	PlayerLives = 3;
	CurrentScore = 0;
	DefaultPawnClass = AShip2::StaticClass();
}

void AAsteroidGameMode::RegisterLevelManager(AAsteroidLevelManager* Manager)
{
	LevelManager = Manager;
}

void AAsteroidGameMode::OnWaveCompleted()
{
	// Reset ship position before next wave spawns
	ResetShipForNewWave();
}

void AAsteroidGameMode::ResetShipForNewWave()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	AShip2* Ship = Cast<AShip2>(PlayerPawn);

	if (Ship)
	{
		// 1. Move ship back to center of screen
		Ship->SetActorLocation(FVector::ZeroVector);
		Ship->SetActorRotation(FRotator::ZeroRotator);

		// 2. Zero out physics momentum
		UPrimitiveComponent* PhysicsComp = Cast<UPrimitiveComponent>(Ship->GetRootComponent());
		if (PhysicsComp)
		{
			PhysicsComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
			PhysicsComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		}

		// 3. Trigger invulnerability frames & visual blinking 
		// TODO: Not implemented yet
		//Ship->StartInvulnerability();
	}
	else if (PlayerLives > 0)
	{
		// If ship was destroyed right at wave end, respawn clean ship
		RespawnPlayer();
	}
}

void AAsteroidGameMode::AddScore(int32 Points)
{
	CurrentScore += Points;
}

void AAsteroidGameMode::OnPlayerDied()
{
	PlayerLives--;

	if (PlayerLives > 0)
	{
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AAsteroidGameMode::RespawnPlayer, RespawnDelay, false);
	}
	else
	{
		OnGameEnded();
	}
}

void AAsteroidGameMode::RespawnPlayer()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		RestartPlayer(PC);
	}
}