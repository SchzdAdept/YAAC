// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputDataConfig.generated.h"

class UInputAction;

UCLASS()
class YAAC_API UInputDataConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> Forward;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> Shoot;
};