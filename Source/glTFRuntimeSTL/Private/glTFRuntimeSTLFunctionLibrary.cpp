// Copyright 2023 Roberto De Ioris.


#include "glTFRuntimeSTLFunctionLibrary.h"

namespace glTFRuntimeSTL
{
	constexpr int32 ASCIIDetectSize = 5; // "solid" string
	constexpr int32 BinaryHeaderSize = 80;
	constexpr int32 BinaryHeaderSizeAndSize = BinaryHeaderSize + 4;
	constexpr int32 BinaryTriangleSize = 50;

	bool LoadToPrimitive(const TArray64<uint8>& Blob, UglTFRuntimeAsset* Asset, FglTFRuntimePrimitive& Primitive, UMaterialInterface* Material, EglTFRuntimeSTLFileMode STLFileMode)
	{
		if ((STLFileMode == EglTFRuntimeSTLFileMode::Auto && Blob.Num() > ASCIIDetectSize && !FMemory::Memcmp(Blob.GetData(), "solid", ASCIIDetectSize)) ||
			STLFileMode == EglTFRuntimeSTLFileMode::ASCII)
		{
			TArray<FString> Tokens;
			FString Token;
			for (int32 Offset = 0; Offset < Blob.Num(); Offset++)
			{
				char Char = static_cast<char>(Blob[Offset]);
				if (Char == 0 || Char == '\r' || Char == '\n' || Char == ' ' || Char == '\t')
				{
					if (!Token.IsEmpty())
					{
						Tokens.Add(Token);
						Token.Empty();
					}
				}
				else
				{
					Token += Char;
				}
			}

			int32 TokenIndex = 0;
			int32 VertexState = 0; // 0 to 2

			while (TokenIndex < Tokens.Num())
			{
				Token = Tokens[TokenIndex++];
				if (Token == "normal")
				{
					if (TokenIndex + 3 > Tokens.Num())
					{
						return false;
					}
					else
					{
						float NormalX = FCString::Atof(*Tokens[TokenIndex++]);
						float NormalY = FCString::Atof(*Tokens[TokenIndex++]);
						float NormalZ = FCString::Atof(*Tokens[TokenIndex++]);
						FVector Normal = Asset->GetParser()->TransformVector(FVector(NormalX, NormalY, NormalZ));
						Primitive.Normals.Add(Normal);
						Primitive.Normals.Add(Normal);
						Primitive.Normals.Add(Normal);
					}
				}
				else if (Token == "vertex")
				{
					if (TokenIndex + 3 > Tokens.Num())
					{
						return false;
					}
					else
					{
						float VertexX = FCString::Atof(*Tokens[TokenIndex++]);
						float VertexY = FCString::Atof(*Tokens[TokenIndex++]);
						float VertexZ = FCString::Atof(*Tokens[TokenIndex++]);
						Primitive.Positions.Add(Asset->GetParser()->TransformPosition(FVector(VertexX, VertexY, VertexZ)));
					}
				}
			}
		}
		else
		{
			if (Blob.Num() < glTFRuntimeSTL::BinaryHeaderSizeAndSize)
			{
				return false;
			}

			int64 Offset = glTFRuntimeSTL::BinaryHeaderSize;

			const uint32* TrianglesNum = reinterpret_cast<const uint32*>(Blob.GetData() + Offset);
			Offset += sizeof(uint32);

			Primitive.Positions.Reserve(*TrianglesNum * 3);
			Primitive.Normals.Reserve(*TrianglesNum * 3);

			for (uint32 ProcessedTriangles = 0; ProcessedTriangles < *TrianglesNum; ProcessedTriangles++)
			{
				if (Offset + glTFRuntimeSTL::BinaryTriangleSize <= Blob.Num())
				{
					const float* NormalX = reinterpret_cast<const float*>(Blob.GetData() + Offset);
					Offset += sizeof(float);
					const float* NormalY = reinterpret_cast<const float*>(Blob.GetData() + Offset);
					Offset += sizeof(float);
					const float* NormalZ = reinterpret_cast<const float*>(Blob.GetData() + Offset);
					Offset += sizeof(float);

					FVector Normal = Asset->GetParser()->TransformVector(FVector(*NormalX, *NormalY, *NormalZ));
					Primitive.Normals.Add(Normal);
					Primitive.Normals.Add(Normal);
					Primitive.Normals.Add(Normal);

					for (int32 VertexIndex = 0; VertexIndex < 3; VertexIndex++)
					{
						const float* VertexX = reinterpret_cast<const float*>(Blob.GetData() + Offset);
						Offset += sizeof(float);
						const float* VertexY = reinterpret_cast<const float*>(Blob.GetData() + Offset);
						Offset += sizeof(float);
						const float* VertexZ = reinterpret_cast<const float*>(Blob.GetData() + Offset);
						Offset += sizeof(float);
						Primitive.Positions.Add(Asset->GetParser()->TransformPosition(FVector(*VertexX, *VertexY, *VertexZ)));
					}

					const uint16 AttributesBytesCount = *(reinterpret_cast<const uint16*>(Blob.GetData() + Offset));
					if (AttributesBytesCount & 0x8000)
					{
						float B = (AttributesBytesCount & 0x1f) / 255.0f;
						float G = ((AttributesBytesCount >> 5) & 0x1f) / 255.0f;
						float R = ((AttributesBytesCount >> 10) & 0x1f) / 255.0f;
						const FLinearColor Color = FLinearColor(R, G, B, 1);
						Primitive.Colors.Add(Color);
						Primitive.Colors.Add(Color);
						Primitive.Colors.Add(Color);
					}
					else
					{
						Primitive.Colors.Add(FLinearColor::White);
						Primitive.Colors.Add(FLinearColor::White);
						Primitive.Colors.Add(FLinearColor::White);
					}
					Offset += sizeof(uint16);
				}
				else
				{
					return false;
				}
			}
		}

		// fill indices
		Primitive.Indices.AddUninitialized(Primitive.Positions.Num());
		for (int32 VertexIndex = 0; VertexIndex < Primitive.Positions.Num(); VertexIndex++)
		{
			Primitive.Indices[VertexIndex] = VertexIndex;
		}

		Primitive.Material = Material;
		if (!Primitive.Material)
		{
			Primitive.Material = UMaterial::GetDefaultMaterial(MD_Surface);
		}

		return true;
	}
};

bool UglTFRuntimeSTLFunctionLibrary::LoadSTLAsRuntimeLOD(UglTFRuntimeAsset* Asset, UMaterialInterface* Material, EglTFRuntimeSTLFileMode STLFileMode, FglTFRuntimeMeshLOD& RuntimeLOD)
{
	if (!Asset)
	{
		return false;
	}

	const TArray64<uint8>& Blob = Asset->GetParser()->GetBlob();

	RuntimeLOD.Empty();

	FglTFRuntimePrimitive Primitive;

	if (!glTFRuntimeSTL::LoadToPrimitive(Blob, Asset, Primitive, Material, STLFileMode))
	{
		return false;
	}

	RuntimeLOD.bHasNormals = true;
	RuntimeLOD.Primitives.Add(MoveTemp(Primitive));

	return true;
}

bool UglTFRuntimeSTLFunctionLibrary::LoadSTLFromArchiveAsRuntimeLOD(UglTFRuntimeAsset* Asset, const FString& Name, UMaterialInterface* Material, EglTFRuntimeSTLFileMode STLFileMode, FglTFRuntimeMeshLOD& RuntimeLOD)
{
	if (!Asset)
	{
		return false;
	}

	TArray64<uint8> Blob;
	if (!Asset->GetParser()->GetBlobByName(Name, Blob))
	{
		return false;
	}

	FglTFRuntimePrimitive Primitive;

	if (!glTFRuntimeSTL::LoadToPrimitive(Blob, Asset, Primitive, Material, STLFileMode))
	{
		return false;
	}

	RuntimeLOD.bHasNormals = true;
	RuntimeLOD.Primitives.Add(MoveTemp(Primitive));

	return true;
}

void UglTFRuntimeSTLFunctionLibrary::LoadSTLAsRuntimeLODAsync(UglTFRuntimeAsset* Asset, UMaterialInterface* Material, EglTFRuntimeSTLFileMode STLFileMode, const FglTFRuntimeMeshLODAsync& AsyncCallback)
{
	if (!Asset)
	{
		AsyncCallback.ExecuteIfBound(false, FglTFRuntimeMeshLOD());
		return;
	}

	Asset->GetParser()->LoadAsRuntimeLODAsync([Asset, Material, STLFileMode](FglTFRuntimeMeshLOD& RuntimeLOD)
		{
			const TArray64<uint8>& Blob = Asset->GetParser()->GetBlob();

			FglTFRuntimePrimitive Primitive;

			if (!glTFRuntimeSTL::LoadToPrimitive(Blob, Asset, Primitive, Material, STLFileMode))
			{
				return false;
			}

			RuntimeLOD.bHasNormals = true;
			RuntimeLOD.Primitives.Add(MoveTemp(Primitive));

			return true;
		}, AsyncCallback);
}

void UglTFRuntimeSTLFunctionLibrary::LoadSTLFromArchiveAsRuntimeLODAsync(UglTFRuntimeAsset* Asset, const FString& Name, UMaterialInterface* Material, EglTFRuntimeSTLFileMode STLFileMode, const FglTFRuntimeMeshLODAsync& AsyncCallback)
{
	if (!Asset)
	{
		AsyncCallback.ExecuteIfBound(false, FglTFRuntimeMeshLOD());
		return;
	}

	Asset->GetParser()->LoadAsRuntimeLODAsync([Asset, Name, Material, STLFileMode](FglTFRuntimeMeshLOD& RuntimeLOD)
		{
			TArray64<uint8> Blob;
			if (!Asset->GetParser()->GetBlobByName(Name, Blob))
			{
				return false;
			}

			FglTFRuntimePrimitive Primitive;

			if (!glTFRuntimeSTL::LoadToPrimitive(Blob, Asset, Primitive, Material, STLFileMode))
			{
				return false;
			}

			RuntimeLOD.bHasNormals = true;
			RuntimeLOD.Primitives.Add(MoveTemp(Primitive));

			return true;
		}, AsyncCallback);
}