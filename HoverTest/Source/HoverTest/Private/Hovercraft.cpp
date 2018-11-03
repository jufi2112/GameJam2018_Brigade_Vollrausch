// Fill out your copyright notice in the Description page of Project Settings.

#include "Hovercraft.h"
#include "Classes/Components/InputComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Components/PrimitiveComponent.h"
#include "Classes/Engine/World.h"
#include "HoverThruster.h"
#include "HovercraftPlayerController.h"
#include "Classes/Components/SceneComponent.h"
#include "MomentumThruster.h"


// Sets default values
AHovercraft::AHovercraft()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AHovercraft::BeginPlay()
{
	Super::BeginPlay();

	// needs to be called before IsFallingArray.Init()
	InitializeThrusters();

	IsFallingArray.Init(false, NumberOfThrusters);
	
}

void AHovercraft::MoveForward(float Value)
{
	if (!StaticMesh) { return; }
	Value = FMath::Clamp<float>(Value, -1.f, 1.f);
	FVector ForceDirection = GetActorForwardVector();
	if (GetIsFalling())
	{
		ForceDirection.Z = 0.f;
		ForceDirection = ForceDirection.GetSafeNormal();
	}

	FVector ForceToApply = ForceDirection * ForwardAcceleration * Value;
	if (GetIsFalling())
	{
		ForceToApply *= ForwardAirborneControl;
	}

	StaticMesh->AddForce(ForceToApply);

	//// Momentum force
	//if (!BackMomentumThruster) { return; }
	//if (Value <= 0)
	//{
	//	BackMomentumThruster->ApplyForce();
	//}
}

void AHovercraft::MoveRight(float Value)
{
	if (!StaticMesh) { return; }
	Value = FMath::Clamp<float>(Value, -1.f, 1.f);
	FVector ForceDirection = GetActorRightVector();
	if (GetIsFalling())
	{
		ForceDirection.Z = 0;
		ForceDirection = ForceDirection.GetSafeNormal();
	}

	FVector ForceToApply = ForceDirection * SidewardAcceleration * Value;
	if (GetIsFalling())
	{
		ForceToApply *= SidewardsAirborneControl;
	}

	StaticMesh->AddForce(ForceToApply);
}

void AHovercraft::RotateRight(float Value)
{
	if (!StaticMesh || !GetWorld()) { return; }
	/*Value = FMath::Clamp<float>(Value, -1.f, 1.f);
	FRotator Rotator = FRotator(0, RotationSpeed * GetWorld()->DeltaTimeSeconds * Value, 0);
	StaticMesh->AddWorldRotation(Rotator);*/

	Value = FMath::Clamp<float>(Value, -1.f, 1.f);
	// momentum force
	if (!RightRotationPoint || !LeftRotationPoint) { return; }
	FVector ForceToApply = StaticMesh->GetRightVector() * Value * RotationForce;
	if (Value > 0)
	{
		StaticMesh->AddForceAtLocation(ForceToApply, LeftRotationPoint->GetComponentLocation());
	}
	else
	{
		StaticMesh->AddForceAtLocation(ForceToApply, RightRotationPoint->GetComponentLocation());
	}
}


void AHovercraft::SetStaticMeshReference(UStaticMeshComponent * MeshToSet)
{
	StaticMesh = MeshToSet;
}

void AHovercraft::CheckIsUpsideDown()
{
	if (!StaticMesh) { return; }
	// raytrace up
	FVector StartLocation = StaticMesh->GetComponentLocation();
	FVector EndLocation = StartLocation + StaticMesh->GetUpVector() * UpsideDownTraceLength;

	FCollisionQueryParams CollisionParams = FCollisionQueryParams::DefaultQueryParam;
	CollisionParams.AddIgnoredActor(this);

	FHitResult HitResult;

	/* Debug information */
	/*const FName TraceTag = FName("UpsideDownTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;*/
	const FName TraceTag = FName("HoverTrace");	// same name as in HoverThruster so we can draw both traces simultaneously
	CollisionParams.TraceTag = TraceTag;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility, CollisionParams))
	{
		bIsUpsideDown = true;
	}
	else
	{
		bIsUpsideDown = false;
	}
}

void AHovercraft::InitializeThrusters()
{
	TArray<UActorComponent*> Thrusters = GetComponentsByClass(UHoverThruster::StaticClass());
	NumberOfThrusters = Thrusters.Num();
	for (const auto& Thruster : Thrusters)
	{
		HoverThrusters.Add(Cast<UHoverThruster>(Thruster));
	}
}

float AHovercraft::GetHovercraftMass()
{
	float Mass = 0.f;
	TArray<UPrimitiveComponent*> Components;
	GetComponents(Components);
	for (auto Iter = Components.CreateConstIterator(); Iter; ++Iter)
	{
		UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(*Iter);
		if (Component)
		{
			if (Component->IsSimulatingPhysics())
			{
				Mass += Component->GetMass();
			}
		}
	}

	return Mass;
}

void AHovercraft::SetIsFalling(bool IsFalling, int32 ThrustID)
{
	IsFallingArray[ThrustID - 1] = IsFalling;
}

bool AHovercraft::GetIsFalling()
{
	bool Falling = true;
	for (bool Value : IsFallingArray)
	{
		if (!Value)
		{
			Falling = false;
		}
	}

	return Falling;
}

void AHovercraft::ResetHovercraft(USceneComponent* AzimuthGimbal)
{
	if (!StaticMesh || !AzimuthGimbal) { return; }

	// Reset Hovercraft

	//// get AzimuthGimbal location
	//FVector AzimuthGimbalLocation = AzimuthGimbal->GetComponentLocation();
	//AzimuthGimbalLocation.Z += ResetHeightModificator;

	//// get current AzimuthGimbal rotation
	//FRotator AzimuthGimbalRotator = AzimuthGimbal->GetComponentRotation();

	//// set roll values to 0
	//AzimuthGimbalRotator.Roll = 0.f;

	// transform hovercraft

	// translation
	FVector ResetLocation;
	FRotator ResetRotation;
	AHovercraftPlayerController* PC = Cast<AHovercraftPlayerController>(GetController());
	if (PC)
	{
		ResetLocation = PC->GetResetPosition();
		if (HoverThrusters[0])
		{
			ResetLocation.Z = HoverThrusters[0]->HoverHeight;
		}
		ResetLocation.Z += ResetHeightModificator;
		ResetRotation = FRotator(0.f, PC->GetResetYaw(), 0.f);
	}
	else
	{
		ResetLocation = StaticMesh->GetComponentLocation();
		ResetLocation.Z += ResetHeightModificator;
		ResetRotation = GetActorRotation();
		ResetRotation.Pitch = 0.f;
		ResetRotation.Roll = 0.f;
		UE_LOG(LogTemp, Warning, TEXT("Could not find PlayerController in %s, used local values instead"), *GetName());
	}

	// set velocity to zero
	StaticMesh->SetPhysicsLinearVelocity(FVector(0.f, 0.f, 0.f));

	// apply values
	SetActorLocation(ResetLocation);
	SetActorRotation(ResetRotation);


	//// set old AzimuthGimbal and SpringArm rotations
	//AzimuthGimbal->SetWorldLocation(AzimuthGimbalLocation);
	//AzimuthGimbal->SetWorldRotation(AzimuthGimbalRotator);

}

void AHovercraft::ToggleShouldHover()
{
	bShouldHover = !bShouldHover;

	for (const auto& Thruster : HoverThrusters)
	{
		Thruster->SetShouldHover(bShouldHover);
	}
}

void AHovercraft::ResetHoverHeight()
{
	for (const auto& Thruster : HoverThrusters)
	{
		Thruster->ResetHoverValues();
	}
}

void AHovercraft::ChangeHoverHeightBySteps(int32 Steps)
{
	for (const auto& Thruster : HoverThrusters)
	{
		Thruster->ChangeHoverHeightBySteps(Steps);
	}
}

void AHovercraft::ToggleDrawDebugTraces()
{
	bDrawDebugTraces = !bDrawDebugTraces;
	const FName DebugTraceToDraw = bDrawDebugTraces ? FName("HoverTrace") : NAME_None;
	GetWorld()->DebugDrawTraceTag = DebugTraceToDraw;
}

bool AHovercraft::GetStaticMeshLocation(FVector& Location)
{
	if (!StaticMesh) { return false; }
	Location = StaticMesh->GetComponentLocation();
	return true;
}

int32 AHovercraft::GetSpeed()
{
	FVector Velocity = GetVelocity() * StaticMesh->GetForwardVector();
	return int32(Velocity.Size() / 100.f);
}

void AHovercraft::SetMomentumThrusterReferences(UMomentumThruster * RightMomentumReference, UMomentumThruster * LeftMomentumReference, UMomentumThruster * BackMomentumReference)
{
	RightMomentumThruster = RightMomentumReference;
	LeftMomentumThruster = LeftMomentumReference;
	BackMomentumThruster = BackMomentumReference;
}

void AHovercraft::SetRotationPointReferences(USceneComponent * RightRotationPointReference, USceneComponent * LeftRotationPointReference)
{
	RightRotationPoint = RightRotationPointReference;
	LeftRotationPoint = LeftRotationPointReference;
}

// Called every frame
void AHovercraft::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!StaticMesh) { return; }

	if (GetIsFalling())
	{
		StaticMesh->AddForce(FVector(0, 0, -1) * DownwardForce);
	}

	// check if hovercraft is upside down
	CheckIsUpsideDown();

}

// Called to bind functionality to input
void AHovercraft::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// setup input bindings in blueprint for soft references

}

