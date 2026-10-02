#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VisualWrapComponent.generated.h"

// Forward declaration to avoid unnecessary header bloat
class UNiagaraComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YAAC_API UVisualWrapComponent : public UActorComponent
{
	GENERATED_BODY()

public:    
	UVisualWrapComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:    
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(EditAnywhere, Category = "Screen Visuals")
	float PlayAreaWidth = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Screen Visuals")
	float PlayAreaHeight = 2100.0f;

	float MaxX;
	float MinX;
	float MaxY;
	float MinY;

	UPROPERTY(EditAnywhere, Category = "Screen Visuals")
	float DetectionBuffer = 300.0f; 

	UPROPERTY()
	AActor* GhostX = nullptr;
	UPROPERTY()
	AActor* GhostY = nullptr;
	UPROPERTY()
	AActor* GhostCorner = nullptr;

	void UpdateGhosts();
	AActor* SpawnGhostClone();
	void DestroyGhost(AActor*& GhostActor);
};