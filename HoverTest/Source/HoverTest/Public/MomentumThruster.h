// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MomentumThruster.generated.h"

class UStaticMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOVERTEST_API UMomentumThruster : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMomentumThruster();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UStaticMeshComponent* StaticMesh = nullptr;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// force with which we will generate "momentum"
	UPROPERTY(EditAnywhere, Category = Momentum)
	float MomentumThrust = 100000.f;

	// should the momentum stop if the speed is low enough
	UPROPERTY(EditAnywhere, Category = Momentum)
	bool bStopAtSpeed = false;

	// the speed under which no momentum is applied
	UPROPERTY(EditAnywhere, Category = Momentum)
	int32 SpeedToStopAt = 20;

	UFUNCTION(BlueprintCallable)
	void ApplyForce();

	UFUNCTION(BlueprintCallable)
	void SetStaticMeshReference(UStaticMeshComponent* ReferenceToSet);

		
	
};
