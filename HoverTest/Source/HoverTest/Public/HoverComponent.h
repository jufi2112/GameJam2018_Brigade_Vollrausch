// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HoverComponent.generated.h"

class UStaticMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOVERTEST_API UHoverComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHoverComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UStaticMeshComponent* StaticMesh = nullptr;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Distance in cm the actor should hover over the ground
	UPROPERTY(EditAnywhere)
	float HoverHeight = 100.f;

	// should the actor hover
	UPROPERTY(EditAnywhere)
	bool bShouldHover = true;

	// the strength to apply to the actor to keep it hovering
	UPROPERTY(EditAnywhere)
	float HoverForceMaxStrength = 200000.f;

	UFUNCTION(BlueprintCallable, Category = Setup)
	void SetStaticMeshReference(UStaticMeshComponent* StaticMeshToSet);

private:

	void OldHover();

	void Hover();



		
	
};
