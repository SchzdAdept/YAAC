#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "GameFramework/Actor.h"
#include "Asteroid.generated.h"

class UStaticMeshComponent;

// Enum to define asteroid size
UENUM(BlueprintType)
enum class EAsteroidSize : uint8
{
	Large   UMETA(DisplayName = "Large"),
	Medium  UMETA(DisplayName = "Medium"),
	Small   UMETA(DisplayName = "Small")
};

UCLASS()
class YAAC_API AAsteroid : public AActor
{
	GENERATED_BODY()
    
public:   	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* ExplosionEffect;
	
	void PlayDeathEffects();
	
	AAsteroid();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* AsteroidMesh;

	// Size configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Properties")
	EAsteroidSize AsteroidSize = EAsteroidSize::Large;

	// Call this after spawning to set size and random direction/velocity
	void InitializeAsteroid(EAsteroidSize NewSize, FVector Direction);

	// Call this when shot by a bullet
	void BreakApart();

protected:
	virtual void BeginPlay() override;

	// Random rotation and linear movement speeds
	FVector MovementDirection;
	float MoveSpeed = 200.0f;
	FRotator RotationRate;

	// Overlap event handler
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);
};