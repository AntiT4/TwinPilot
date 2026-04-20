#pragma once

#include "CoreMinimal.h"
#include "JsonObjectWrapper.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "DigitalTwinJsonBlueprintLibrary.generated.h"

UCLASS()
class TWINPILOT_API UDigitalTwinJsonBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintPure,
		CustomThunk,
		Category = "TwinPilot|Json",
		meta = (DisplayName = "Set Json Field", CustomStructureParam = "Value", AutoCreateRefTerm = "JsonObject,Value"))
	static UPARAM(DisplayName = "Success") bool SetJsonFieldPure(
		const FJsonObjectWrapper& JsonObject,
		const FString& FieldName,
		const int32& Value,
		UPARAM(DisplayName = "JsonObject") FJsonObjectWrapper& OutJsonObject);
	DECLARE_FUNCTION(execSetJsonFieldPure);

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Json", meta = (DisplayName = "Make Empty Json Object"))
	static FJsonObjectWrapper MakeEmptyJsonObject();

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Json", meta = (DisplayName = "Set Json String Field", AutoCreateRefTerm = "JsonObject,Value"))
	static UPARAM(DisplayName = "Success") bool SetJsonStringFieldPure(
		const FJsonObjectWrapper& JsonObject,
		const FString& FieldName,
		const FString& Value,
		UPARAM(DisplayName = "JsonObject") FJsonObjectWrapper& OutJsonObject);

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Json", meta = (DisplayName = "Json Object To String"))
	static UPARAM(DisplayName = "Success") bool JsonObjectToStringPure(
		const FJsonObjectWrapper& JsonObject,
		UPARAM(DisplayName = "String") FString& OutJsonString);

private:
	static bool CloneJsonObject(const FJsonObjectWrapper& SourceObject, FJsonObjectWrapper& OutJsonObject);
	static bool SetJsonFieldFromProperty(
		const FString& FieldName,
		FProperty* SourceProperty,
		const void* SourceValuePtr,
		FJsonObjectWrapper& TargetObject);
	static bool SerializeJsonObject(FJsonObjectWrapper& JsonObject);
};
