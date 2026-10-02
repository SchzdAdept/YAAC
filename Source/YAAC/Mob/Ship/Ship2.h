#pragma once


#include "NiagaraFunctionLibrary.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h" // Required for Enhanced Input values
#include "Ship2.generated.h"

class UInputAction;
class UArrowComponent;
class ABullet;

UCLASS()
class YAAC_API AShip2 : public APawn
{
	GENERATED_BODY()

public:
	AShip2();

	virtual void PossessedBy(AController* NewController) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* ExplosionEffect;
	
protected:
	
	void PlayDeathEffects();
	
	virtual void BeginPlay() override;

public:

	UFUNCTION()
	void OnShipOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
					   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
					   bool bFromSweep, const FHitResult& SweepResult);
	
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ShipMesh;

	// --- Enhanced Input Pointers ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* AsteroidMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* ThrustAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* TurnAction;

	// Movement settings
	UPROPERTY(EditAnywhere, Category = "Movement")
	float ThrustAcceleration = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float TurnRate = 500.0f;

protected:
	
	// --- Invulnerability Settings ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invulnerability")
	float InvulnerabilityDuration = 3.0f; // Seconds of invulnerability on spawn

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invulnerability")
	float BlinkInterval = 0.15f; // Speed of mesh blinking

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Invulnerability")
	bool bIsInvulnerable = false;
	
	void StartInvulnerability();
	void EndInvulnerability();
	void ToggleBlinkVisibility();
	
	// Timer Handles
	FTimerHandle InvulnerabilityTimerHandle;
	FTimerHandle BlinkTimerHandle;
	
	
	// Binding functions
	void EnhancedThrust(const FInputActionValue& Value);
	void EnhancedTurn(const FInputActionValue& Value);
	
	// Enhanced Input Fire Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* FireAction;

	// Projectile class to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	TSubclassOf<ABullet> ProjectileClass;

	// Muzzle location marker component
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UArrowComponent* MuzzleLocation;

	// Fire function
	void EnhancedFire();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	float CurrentThrust = 0.0f;

private:

	
	float CurrentTurn = 0.0f;
};