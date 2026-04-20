#include "Libraries/DigitalTwinJsonBlueprintLibrary.h"

#include "Blueprint/BlueprintExceptionInfo.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/UnrealType.h"

#define LOCTEXT_NAMESPACE "DigitalTwinJsonBlueprintLibrary"

DEFINE_FUNCTION(UDigitalTwinJsonBlueprintLibrary::execSetJsonFieldPure)
{
	P_GET_STRUCT_REF(FJsonObjectWrapper, JsonObject);
	P_GET_PROPERTY(FStrProperty, FieldName);

	Stack.StepCompiledIn<FProperty>(nullptr);
	FProperty* SourceProperty = Stack.MostRecentProperty;
	void* SourceValuePtr = Stack.MostRecentPropertyAddress;

	P_GET_STRUCT_REF(FJsonObjectWrapper, OutJsonObject);
	P_FINISH;

	if (!SourceProperty || !SourceValuePtr)
	{
		const FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AccessViolation,
			LOCTEXT("SetJsonFieldPure_MissingInputProperty", "Failed to resolve the input parameter for SetJsonFieldPure."));
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
		*static_cast<bool*>(RESULT_PARAM) = false;
		return;
	}

	bool bResult = false;

	P_NATIVE_BEGIN
	bResult = CloneJsonObject(JsonObject, OutJsonObject);
	if (bResult)
	{
		bResult = SetJsonFieldFromProperty(FieldName, SourceProperty, SourceValuePtr, OutJsonObject);
	}

	if (bResult)
	{
		bResult = SerializeJsonObject(OutJsonObject);
	}
	P_NATIVE_END

	*static_cast<bool*>(RESULT_PARAM) = bResult;
}

FJsonObjectWrapper UDigitalTwinJsonBlueprintLibrary::MakeEmptyJsonObject()
{
	FJsonObjectWrapper JsonObject;
	JsonObject.JsonObject = MakeShared<FJsonObject>();
	JsonObject.JsonString = TEXT("{}");
	return JsonObject;
}

bool UDigitalTwinJsonBlueprintLibrary::SetJsonStringFieldPure(
	const FJsonObjectWrapper& JsonObject,
	const FString& FieldName,
	const FString& Value,
	FJsonObjectWrapper& OutJsonObject)
{
	if (!CloneJsonObject(JsonObject, OutJsonObject))
	{
		return false;
	}

	if (!OutJsonObject.JsonObject.IsValid())
	{
		OutJsonObject.JsonObject = MakeShared<FJsonObject>();
	}

	if (FieldName.IsEmpty())
	{
		FFrame::KismetExecutionMessage(TEXT("FieldName cannot be empty for SetJsonStringFieldPure."), ELogVerbosity::Error);
		return false;
	}

	OutJsonObject.JsonObject->SetStringField(FieldName, Value);
	return SerializeJsonObject(OutJsonObject);
}

bool UDigitalTwinJsonBlueprintLibrary::JsonObjectToStringPure(
	const FJsonObjectWrapper& JsonObject,
	FString& OutJsonString)
{
	OutJsonString.Reset();

	if (JsonObject.JsonObject.IsValid())
	{
		if (!JsonObject.JsonObjectToString(OutJsonString))
		{
			FFrame::KismetExecutionMessage(TEXT("Failed to convert JSON object to string."), ELogVerbosity::Error);
			return false;
		}

		return true;
	}

	const FString TrimmedJsonString = JsonObject.JsonString.TrimStartAndEnd();
	if (TrimmedJsonString.IsEmpty())
	{
		OutJsonString = TEXT("{}");
		return true;
	}

	OutJsonString = TrimmedJsonString;
	return true;
}

bool UDigitalTwinJsonBlueprintLibrary::CloneJsonObject(
	const FJsonObjectWrapper& SourceObject,
	FJsonObjectWrapper& OutJsonObject)
{
	if (SourceObject.JsonObject.IsValid())
	{
		FString JsonString;
		if (!SourceObject.JsonObjectToString(JsonString))
		{
			FFrame::KismetExecutionMessage(TEXT("Failed to clone JSON object."), ELogVerbosity::Error);
			return false;
		}

		return OutJsonObject.JsonObjectFromString(JsonString);
	}

	if (!SourceObject.JsonString.TrimStartAndEnd().IsEmpty())
	{
		return OutJsonObject.JsonObjectFromString(SourceObject.JsonString);
	}

	OutJsonObject = MakeEmptyJsonObject();
	return true;
}

bool UDigitalTwinJsonBlueprintLibrary::SetJsonFieldFromProperty(
	const FString& FieldName,
	FProperty* SourceProperty,
	const void* SourceValuePtr,
	FJsonObjectWrapper& TargetObject)
{
	if (!SourceProperty || !SourceValuePtr)
	{
		return false;
	}

	if (!TargetObject.JsonObject.IsValid())
	{
		TargetObject.JsonObject = MakeShared<FJsonObject>();
	}

	if (FieldName.IsEmpty())
	{
		const FStructProperty* StructProperty = CastField<FStructProperty>(SourceProperty);
		if (StructProperty == nullptr)
		{
			FFrame::KismetExecutionMessage(TEXT("FieldName cannot be empty unless Value is a struct."), ELogVerbosity::Error);
			return false;
		}

		return FJsonObjectConverter::UStructToJsonObject(
			StructProperty->Struct,
			SourceValuePtr,
			TargetObject.JsonObject.ToSharedRef());
	}

	const TSharedPtr<FJsonValue> JsonValue = FJsonObjectConverter::UPropertyToJsonValue(SourceProperty, SourceValuePtr);
	if (!JsonValue.IsValid())
	{
		FFrame::KismetExecutionMessage(
			*FString::Printf(TEXT("Failed to convert field '%s' to a JSON value."), *FieldName),
			ELogVerbosity::Error);
		return false;
	}

	TargetObject.JsonObject->SetField(FieldName, JsonValue);
	return true;
}

bool UDigitalTwinJsonBlueprintLibrary::SerializeJsonObject(FJsonObjectWrapper& JsonObject)
{
	if (!JsonObject.JsonObject.IsValid())
	{
		JsonObject = MakeEmptyJsonObject();
		return true;
	}

	JsonObject.JsonString.Reset();
	if (!FJsonSerializer::Serialize(
		JsonObject.JsonObject.ToSharedRef(),
		TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonObject.JsonString)))
	{
		FFrame::KismetExecutionMessage(TEXT("Failed to serialize JSON object."), ELogVerbosity::Error);
		return false;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
