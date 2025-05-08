// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"
#include "MyDynamicMeshComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MESHDEFORM_API UMyDynamicMeshComponent : public UDynamicMeshComponent
{
	GENERATED_BODY()
public:
	UMyDynamicMeshComponent();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
