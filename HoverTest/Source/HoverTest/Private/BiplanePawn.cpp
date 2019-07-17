// Fill out your copyright notice in the Description page of Project Settings.

#include "BiplanePawn.h"
#include "Classes/Components/PoseableMeshComponent.h"
#include "Classes/Components/StaticMeshComponent.h"


// Sets default values
ABiplanePawn::ABiplanePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABiplanePawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABiplanePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (StaticMesh)
	{
		FVector Torque = (-(StaticMesh->GetPhysicsAngularVelocityInDegrees())) / 0.5f;
		StaticMesh->AddTorqueInDegrees(Torque, NAME_None, true);
		StaticMesh->AddForce(FVector(0.f, 0.f, -1500.f), NAME_None, true);

		CalculateSpeed();

		AnimateBiplane(DeltaTime);
	}

}

// Called to bind functionality to input
void ABiplanePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABiplanePawn::SetPoseableMeshReference(UPoseableMeshComponent * ReferenceToSet)
{
	PoseableMesh = ReferenceToSet;
}

void ABiplanePawn::SetStaticMeshReference(UStaticMeshComponent * ReferenceToSet)
{
	StaticMesh = ReferenceToSet;
}

void ABiplanePawn::ThrottleInput(float Value, EControllerType InputControllerType)
{
	if (Value != 0.f)
	{
		ControllerType = InputControllerType;
	}
	ThrottleAmount += Value;
	ThrottleAmount = FMath::Clamp<float>(ThrottleAmount, 0.f, 100.f);
}


EControllerType ABiplanePawn::GetControllerType() const
{
	return ControllerType;
}

void ABiplanePawn::BiplanePitch(float Value, EControllerType InputControllerType)
{
	if (!StaticMesh) { return; }
	if (Value != 0.f)
	{
		ControllerType = InputControllerType;
	}
	LastPitchAxisValue = Value;
	FVector Torque = FMath::Lerp<FVector, float>(FVector(0.f, 0.f, 0.f), StaticMesh->GetRightVector() * Value * AirControl, 0.1f);
	StaticMesh->AddTorqueInDegrees(Torque, NAME_None, true);
}

void ABiplanePawn::BiplaneRoll(float Value, EControllerType InputControllerType)
{
	if (!StaticMesh) { return; }
	if (Value != 0.f)
	{
		ControllerType = InputControllerType;
	}
	LastRollAxisValue = Value;
	FVector Torque = FMath::Lerp<FVector, float>(FVector(0.f, 0.f, 0.f), StaticMesh->GetForwardVector() * Value * AirControl, 0.1f);
	StaticMesh->AddTorqueInDegrees(Torque, NAME_None, true);
}

void ABiplanePawn::BiplaneYaw(float Value, EControllerType InputControllerType)
{
	if (!StaticMesh) { return; }
	if (Value != 0.f)
	{
		ControllerType = InputControllerType;
	}
	LastYawAxisValue = Value;
	FVector Torque = FMath::Lerp<FVector, float>(FVector(0.f, 0.f, 0.f), StaticMesh->GetUpVector() * Value * AirControl, 0.1f);
	StaticMesh->AddTorqueInDegrees(Torque, NAME_None, true);
}

void ABiplanePawn::CalculateSpeed()
{
	if (!StaticMesh) { return; }
	StaticMesh->SetPhysicsLinearVelocity(FMath::Lerp<FVector, float>(StaticMesh->GetPhysicsLinearVelocity(), StaticMesh->GetForwardVector() * FMath::Clamp<float>(ThrottleAmount * 250.f, 0.f, 25000.f), 0.01f));
	
}

void ABiplanePawn::SetTerrainTrackerReference(UTerrainTrackerComponent * ReferenceToSet)
{
	TTC = ReferenceToSet;
}

UTerrainTrackerComponent * ABiplanePawn::GetTerrainTrackerComponent() const
{
	return TTC;
}

void ABiplanePawn::AnimateBiplane(float DeltaTime)
{
	if (!PoseableMesh) { return; }

	// Propellor
	FRotator Rotation = PoseableMesh->GetBoneRotationByName(FName("Propellor_J"), EBoneSpaces::ComponentSpace);
	Rotation.Yaw = 0.f;
	Rotation.Pitch = 0.f;
	Rotation.Roll += 20.f;
	PoseableMesh->SetBoneRotationByName(FName("Propellor_J"), Rotation, EBoneSpaces::ComponentSpace);

	// Rudder
	Rotation = PoseableMesh->GetBoneRotationByName(FName("Rudder_J"), EBoneSpaces::ComponentSpace);
	Rotation.Yaw = FMath::FInterpTo(Rotation.Yaw, LastYawAxisValue * -35.f, DeltaTime, 10.f);
	Rotation.Pitch = 0.f;
	Rotation.Roll = 0.f;
	PoseableMesh->SetBoneRotationByName(FName("Rudder_J"), Rotation, EBoneSpaces::ComponentSpace);

	// Elevator
	Rotation = PoseableMesh->GetBoneRotationByName(FName("Elevator_L_J"), EBoneSpaces::ComponentSpace);
	Rotation.Yaw = 0.f;
	Rotation.Pitch = FMath::FInterpTo(Rotation.Pitch, FMath::Clamp<float>(LastRollAxisValue * -30.f + LastPitchAxisValue * 30.f, -30.f, 30.f), DeltaTime, 10.f);
	Rotation.Roll = 0.f;
	PoseableMesh->SetBoneRotationByName(FName("Elevator_L_J"), Rotation, EBoneSpaces::ComponentSpace);

	Rotation = PoseableMesh->GetBoneRotationByName(FName("Elevator_R_J"), EBoneSpaces::ComponentSpace);
	Rotation.Yaw = 0.f;
	Rotation.Pitch = FMath::FInterpTo(Rotation.Pitch, FMath::Clamp<float>(LastRollAxisValue * 30.f + LastPitchAxisValue * 30.f, -30.f, 30.f), DeltaTime, 10.f);
	Rotation.Roll = 0.f;
	PoseableMesh->SetBoneRotationByName(FName("Elevator_R_J"), Rotation, EBoneSpaces::ComponentSpace);
}

void ABiplanePawn::SetSpawnSpeed(float Speed)
{
	if (!StaticMesh) { return; }
	StaticMesh->SetPhysicsLinearVelocity(StaticMesh->GetForwardVector() * FMath::Clamp<float>(Speed, 0.f, 25000.f));
}

