// Fill out your copyright notice in the Description page of Project Settings.

#include "HoverThruster.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "Hovercraft.h"


// Sets default values for this component's properties
UHoverThruster::UHoverThruster()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHoverThruster::BeginPlay()
{
	Super::BeginPlay();

	// set reset values
	HoverHeightReset = HoverHeight;
	HoverHeightFallingOffsetReset = HoverHeightFallingOffset;
	TraceLengthReset = TraceLength;

	
}


// Called every frame
void UHoverThruster::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	HoverLogic();

	// TODO this is not the best way, what if we would like to add thrusters to other objects? Maybe an Interface would be better
	AHovercraft* Craft = Cast<AHovercraft>(GetOwner());
	if (Craft)
	{
		Craft->SetIsFalling(bIsFalling, ThrustID);
	}


	
}

void UHoverThruster::SetStaticMeshReference(UStaticMeshComponent * MeshToSet, int32 ThrustID)
{
	StaticMesh = MeshToSet;
	this->ThrustID = ThrustID;
}

bool UHoverThruster::GetIsFalling()
{
	return bIsFalling;
}

void UHoverThruster::SetShouldHover(bool NewHoverState)
{
	bShouldHover = NewHoverState;
}

void UHoverThruster::ResetHoverValues()
{
	HoverHeight = HoverHeightReset;
	HoverHeightFallingOffset = HoverHeightFallingOffsetReset;
	TraceLength = TraceLengthReset;
}

void UHoverThruster::ChangeHoverHeightBySteps(int32 Steps)
{
	HoverHeight += Steps * HoverHeightStepSize;
	HoverHeightFallingOffset += Steps * (HoverHeightStepSize / 10.f);	//TODO check if this value is needed
	TraceLength += Steps * HoverHeightStepSize;
}

void UHoverThruster::HoverLogic()
{
	if (!StaticMesh) { return; }

	// perform a line trace
	FVector StartLocation = GetComponentLocation();
	FVector EndLocation = StartLocation - GetUpVector() * TraceLength;

	FCollisionQueryParams CollisionParams = FCollisionQueryParams::DefaultQueryParam;
	CollisionParams.AddIgnoredActor(GetOwner());

	FHitResult HitResult;

	/* debug information */
	const FName TraceTag = FName("HoverTrace");
	CollisionParams.TraceTag = TraceTag;
	//GetWorld()->DebugDrawTraceTag = TraceTag;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility, CollisionParams))
	{
		if (HitResult.Distance <= HoverHeight)
		{
			if (bShouldHover)
			{
				float Ratio = HitResult.Distance / HoverHeight;
				FVector ForceToApply = FMath::Lerp<float>(MaxHoverForce, 0.f, Ratio) * HitResult.ImpactNormal;
				StaticMesh->AddForceAtLocation(ForceToApply, GetComponentLocation());
				bIsFalling = false;
			}
			else
			{
				// TODO does this make sense? maybe choose other concept?
				bIsFalling = true;
			}

		}
		else if (HitResult.Distance >= HoverHeight + HoverHeightFallingOffset)
		{
			bIsFalling = true;
		}
	}
	else
	{
		bIsFalling = true;
	}
}

