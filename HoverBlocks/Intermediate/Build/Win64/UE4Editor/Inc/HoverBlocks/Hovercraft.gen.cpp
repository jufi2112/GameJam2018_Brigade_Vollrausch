// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "HoverBlocks/Public/Hovercraft.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeHovercraft() {}
// Cross Module References
	HOVERBLOCKS_API UClass* Z_Construct_UClass_AHovercraft_NoRegister();
	HOVERBLOCKS_API UClass* Z_Construct_UClass_AHovercraft();
	ENGINE_API UClass* Z_Construct_UClass_APawn();
	UPackage* Z_Construct_UPackage__Script_HoverBlocks();
// End Cross Module References
	void AHovercraft::StaticRegisterNativesAHovercraft()
	{
	}
	UClass* Z_Construct_UClass_AHovercraft_NoRegister()
	{
		return AHovercraft::StaticClass();
	}
	struct Z_Construct_UClass_AHovercraft_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_AHovercraft_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_APawn,
		(UObject* (*)())Z_Construct_UPackage__Script_HoverBlocks,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_AHovercraft_Statics::Class_MetaDataParams[] = {
		{ "HideCategories", "Navigation" },
		{ "IncludePath", "Hovercraft.h" },
		{ "ModuleRelativePath", "Public/Hovercraft.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_AHovercraft_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AHovercraft>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_AHovercraft_Statics::ClassParams = {
		&AHovercraft::StaticClass,
		DependentSingletons, ARRAY_COUNT(DependentSingletons),
		0x009000A0u,
		nullptr, 0,
		nullptr, 0,
		nullptr,
		&StaticCppClassTypeInfo,
		nullptr, 0,
		METADATA_PARAMS(Z_Construct_UClass_AHovercraft_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_AHovercraft_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_AHovercraft()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_AHovercraft_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(AHovercraft, 1824039699);
	static FCompiledInDefer Z_CompiledInDefer_UClass_AHovercraft(Z_Construct_UClass_AHovercraft, &AHovercraft::StaticClass, TEXT("/Script/HoverBlocks"), TEXT("AHovercraft"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(AHovercraft);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
