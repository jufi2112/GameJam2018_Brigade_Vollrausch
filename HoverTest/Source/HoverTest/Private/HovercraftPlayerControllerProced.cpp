// Fill out your copyright notice in the Description page of Project Settings.

#include "HovercraftPlayerControllerProced.h"
#include "Engine/World.h"
#include "HoverTestGameModeProceduralLevel.h"

void AHovercraftPlayerControllerProced::ToggleBiplane()
{
	UWorld* World = GetWorld();

	if (World)
	{
		AHoverTestGameModeProceduralLevel* GameMode = Cast<AHoverTestGameModeProceduralLevel>(World->GetAuthGameMode());
		if (GameMode)
		{
			// check if player pawn was already created -> necessary
			if (GameMode->WasPlayerPawnCreated())
			{
				// can switch
				bIsBiplaneActive = !bIsBiplaneActive;
				GameMode->ToggleBetweenHovercraftAndBiplane(bIsBiplaneActive);
			}
		}
	}




}
