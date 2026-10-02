#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bullet.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class YAAC_API ABullet : public AActor
{
	GENERATED_BODY()
    
public:    
	ABullet();
	
	virtual void BeginPlay() override;

	FTimerHandle DestroyTimerHandle;

	void SelfDestruct();
	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovement;
};