// Fill out your copyright notice in the Description page of Project Settings.

#include "MomentumThruster.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Hovercraft.h"


// Sets default values for this component's properties
UMomentumThruster::UMomentumThruster()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMomentumThruster::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMomentumThruster::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UMomentumThruster::ApplyForce()
{
	if (!StaticMesh) { return; }
	
	AHovercraft* Craft = Cast<AHovercraft>(GetOwner());
	if (!Craft) { return; }

	if (bStopAtSpeed && Craft->GetSpeed() <= SpeedToStopAt) { return; }
	FVector ForceToApply = GetUpVector() * MomentumThrust;
	StaticMesh->AddForceAtLocation(ForceToApply, GetComponentLocation());
	UE_LOG(LogTemp, Warning, TEXT("%s thrusting."), *GetName());
}

void UMomentumThruster::SetStaticMeshReference(UStaticMeshComponent * ReferenceToSet)
{
	StaticMesh = ReferenceToSet;
}

