#include "MarchedCube.h"
#include "Kismet/GameplayStatics.h"
#include "DynamicMesh/MeshAttributeUtil.h"

AMarchedCube::AMarchedCube() {
	meshComponent = DynamicMeshComponent.Get();
}

/// <summary>
/// Initialises member variables and optionally creates initial mesh with filled top
/// </summary>
void AMarchedCube::BeginPlay() {
	cornerCounts.Init(0, boxNum * boxNum * boxNum);
	InitialiseCorners();
	UStaticMeshComponent* collisionCube = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
	FTransform cubeTransform;
	cubeTransform.SetTranslationAndScale3D(FVector3d((boxNum * boxSize) / 2.0), FVector3d((boxNum * boxSize) / 100.0));
	collisionCube->SetRelativeTransform(cubeTransform);
	if (fillTop) {
		// Generate initial mesh
		CarveMesh(true, true);
	}
}

/// <summary>
/// Updates mesh features
/// </summary>
/// <param name="isSubtractive">Whether to subtract or add to mesh</param>
/// <param name="ignoreCarveSphere">Whether carveSphere should affect mesh</param>
void AMarchedCube::CarveMesh(bool isSubtractive, bool ignoreCarveSphere) {
	if (!carveSphere) {
		TArray<AActor*> actorsWithTag;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Character"), actorsWithTag);
		carveSphere = Cast<UStaticMeshComponent>(actorsWithTag[0]->GetComponentsByTag(UStaticMeshComponent::StaticClass(), FName("CarveSphere"))[0]);
	}
	int cornerTotal = 0;
	TArray<int> triIndexes;
	for (int iZ = 0; iZ < boxNum; iZ++) {
		for (int iY = 0; iY < boxNum; iY++) {
			for (int iX = 0; iX < boxNum; iX++) {
				triIndexes.Add(CalculateTriIndex(FVector3d(iX, iY, iZ) * boxSize, cornerTotal, isSubtractive, !ignoreCarveSphere));
				cornerTotal += 8;
			}
		}
	}
	UpdateMesh(triIndexes);
}

/// <summary>
/// Update mesh vertices, triangles, collision primitives, and UVs (depending on addDuplicateVertexUVs)
/// </summary>
/// <param name="pTriIndexes">Array of triTable indexes to describe the full mesh</param>
void AMarchedCube::UpdateMesh(TArray<int> pTriIndexes) {
	mesh = meshComponent->GetDynamicMesh()->GetMeshPtr();
	mesh->Clear();
	if (addDuplicateVertexUVs) {
		mesh->EnableAttributes();
		mesh->EnableVertexUVs(FVector2f::Zero());
		mesh->EnableVertexNormals(FVector3f::Zero());
	}
	FKAggregateGeom boxCollision;
	int indexOffset = 0;
	int boxIndex = 0;
	int vertexCount = 0;
	for (int iZ = 0; iZ < boxNum; iZ++) {
		for (int iY = 0; iY < boxNum; iY++) {
			for (int iX = 0; iX < boxNum; iX++) {
				// Either use shared vertices, or duplicate vertices but with UV coordinates
				if (!addDuplicateVertexUVs) {
					// Populate vertex map to point to shared vertices
					for (int i = 0; i < 12; i++) {
						if (iZ > 0 && (i == 0 || i == 4 || i == 8 || i == 9)) {
							vertexMap.Add(i + indexOffset, vertexMap[vertexPairsZ[i] + (indexOffset - (12 * boxNum * boxNum))]);
						}
						else if (iY > 0 && (i == 0 || i == 1 || i == 2 || i == 3)) {
							vertexMap.Add(i + indexOffset, vertexMap[vertexPairsY[i] + (indexOffset - (12 * boxNum))]);
						}
						else if (iX > 0 && (i == 3 || i == 7 || i == 8 || i == 11)) {
							vertexMap.Add(i + indexOffset, vertexMap[vertexPairsX[i] + (indexOffset - 12)]);
						}
						else {
							vertexMap.Add(i + indexOffset, -1);
						}
					}
					// Add vertices and triangles
					for (int i = 0; i < 5; i++) {
						if (triTable[pTriIndexes[boxIndex]][i * 3] != -1) {
							for (int v = 0; v < 3; v++) {
								if (vertexMap[triTable[pTriIndexes[boxIndex]][(i * 3) + v] + indexOffset] == -1) {
									vertexMap[triTable[pTriIndexes[boxIndex]][(i * 3) + v] + indexOffset] = vertexCount;
									mesh->AppendVertex((vertices[triTable[pTriIndexes[boxIndex]][(i * 3) + v]] + FVector3d(iX, iY, iZ)) * boxSize);
									vertexCount++;
								}
							}
							mesh->AppendTriangle(
								vertexMap[triTable[pTriIndexes[boxIndex]][i * 3] + indexOffset],
								vertexMap[triTable[pTriIndexes[boxIndex]][(i * 3) + 2] + indexOffset],
								vertexMap[triTable[pTriIndexes[boxIndex]][(i * 3) + 1] + indexOffset]
							);
						}
						else {
							break;
						}
					}
				}
				else {
					// Add triangles with unique vertices
					for (int i = 0; i < 5; i++) {
						if (triTable[pTriIndexes[boxIndex]][i * 3] != -1) {
							mesh->AppendVertex((vertices[triTable[pTriIndexes[boxIndex]][i * 3]] + FVector3d(iX, iY, iZ)) * boxSize);
							mesh->AppendVertex((vertices[triTable[pTriIndexes[boxIndex]][(i * 3) + 2]] + FVector3d(iX, iY, iZ)) * boxSize);
							mesh->AppendVertex((vertices[triTable[pTriIndexes[boxIndex]][(i * 3) + 1]] + FVector3d(iX, iY, iZ)) * boxSize);
							vertexCount += 3;
							mesh->AppendTriangle(
								vertexCount - 3,
								vertexCount - 2,
								vertexCount - 1
							);
							// Calculate and add UV coordinates
							FVector2f uvs[3];
							FVector3f normal;
							CalculateUVsAndNormal((vertices[triTable[pTriIndexes[boxIndex]][i * 3]] + FVector3d(iX, iY, iZ)) * boxSize, (vertices[triTable[pTriIndexes[boxIndex]][(i * 3) + 2]] + FVector3d(iX, iY, iZ)) * boxSize, (vertices[triTable[pTriIndexes[boxIndex]][(i * 3) + 1]] + FVector3d(iX, iY, iZ)) * boxSize, uvs, &normal);
							mesh->SetVertexUV(vertexCount - 3, uvs[0]);
							mesh->SetVertexUV(vertexCount - 2, uvs[1]);
							mesh->SetVertexUV(vertexCount - 1, uvs[2]);

							mesh->SetVertexNormal(vertexCount - 3, normal);
							mesh->SetVertexNormal(vertexCount - 2, normal);
							mesh->SetVertexNormal(vertexCount - 1, normal);
						}
						else {
							break;
						}
					}
				}
				// Add collision cube
				if (cornerCounts[boxIndex] >= 4 && cornerCounts[boxIndex] <= 6) {
					FKBoxElem newBoxCol;
					newBoxCol.Center = FVector3d(iX + 0.5f, iY + 0.5f, iZ + 0.5f) * boxSize;
					newBoxCol.X = boxSize;
					newBoxCol.Y = boxSize;
					newBoxCol.Z = boxSize;
					boxCollision.BoxElems.Add(newBoxCol);
					//DrawDebugBox(GetWorld(), newBoxCol.Center + GetTransform().GetLocation(), FVector(boxSize * 0.5f), FColor(255, 255, 0), false, 0.3f);
				}
				// Check if corner collision needed
				else if (cornerCounts[boxIndex] >= 1 && cornerCounts[boxIndex] < 3) {
					bool isXLim = (iX == boxNum - 1 && cornerValues[cornerMap[(boxIndex * 8) + 1]] == 1);
					bool isYLim = (iY == boxNum - 1 && cornerValues[cornerMap[(boxIndex * 8) + 3]] == 1);
					bool isZLim = (iZ == boxNum - 1 && cornerValues[cornerMap[(boxIndex * 8) + 4]] == 1);
					if (cornerValues[cornerMap[boxIndex * 8]] == 1 || isXLim || isYLim || isZLim) {
						if (TryCornerCount(boxIndex - 1) >= 5 ||
							TryCornerCount(boxIndex + 1) >= 5 ||
							TryCornerCount(boxIndex - (boxNum)) >= 5 ||
							TryCornerCount(boxIndex + (boxNum)) >= 5 ||
							TryCornerCount(boxIndex - (boxNum * boxNum)) >= 5 ||
							TryCornerCount(boxIndex + (boxNum * boxNum)) >= 5) {
						}
						else {
							// Add small corner collision cube
							FKBoxElem newBoxCol;
							newBoxCol.Center = FVector3d(iX + (isXLim ? 1 : 0), iY + (isYLim ? 1 : 0), iZ + (isZLim ? 1 : 0)) * boxSize;
							newBoxCol.X = boxSize * 0.5f;
							newBoxCol.Y = boxSize * 0.5f;
							newBoxCol.Z = boxSize * 0.5f;
							boxCollision.BoxElems.Add(newBoxCol);
							//DrawDebugBox(GetWorld(), newBoxCol.Center + GetTransform().GetLocation(), FVector(boxSize * 0.25f), FColor(0, 0, 255), false, 0.3f);
						}
					}
				}
				indexOffset += 12;
				boxIndex++;
			}
		}
	}
	if (addDuplicateVertexUVs) {
		UE::Geometry::CopyVertexUVsToOverlay(*mesh, *(mesh->Attributes()->PrimaryUV()));
	}
	mesh->AllocateTriangleGroup();
	meshComponent->NotifyMeshUpdated();
	meshComponent->SetSimpleCollisionShapes(boxCollision, true);
}

/// <summary>
/// Calculates index of triTable to describe a single box's corners as a mesh
/// </summary>
/// <param name="offset">World space offset of box to calculate</param>
/// <param name="indexOffset">Number of previously calculated corners</param>
/// <param name="isSubtractive">Whether to subtract or add corner values</param>
/// <param name="testDistance">Whether to update corner values based on distance to carveSphere</param>
/// <returns></returns>
int AMarchedCube::CalculateTriIndex(FVector3d offset, int indexOffset, bool isSubtractive, bool testDistance) {
	FVector meshLocation = GetTransform().GetLocation();
	FVector carveLocation = carveSphere->GetComponentTransform().GetLocation();
	int triIndex = 0;
	int cornerIndex = 0;
	int cornerCount = 0;
	for (int iZ	 = 0; iZ < 2 * boxSize; iZ += boxSize) {
		for (int iY = 0; iY < 2 * boxSize; iY += boxSize) {
			for (int iX = 0; iX < 2 * boxSize; iX += boxSize) {
				if (testDistance) {
					if ((offset.Z > 0 && (cornerIndex == 0 || cornerIndex == 1 || cornerIndex == 2 || cornerIndex == 3)) ||
						(offset.Y > 0 && (cornerIndex == 0 || cornerIndex == 1 || cornerIndex == 4 || cornerIndex == 5)) ||
						(offset.X > 0 && (cornerIndex == 0 || cornerIndex == 3 || cornerIndex == 4 || cornerIndex == 7))) {
						// Distance from this point calculated by a previous box
					}
					else {
						// Set corner state based off distance to carveSphere
						double distance = FVector::Distance(carveLocation, FVector3d(meshLocation.X + ((iY == 0) ? iX : boxSize - iX), meshLocation.Y + iY, meshLocation.Z + iZ) + offset);
						if (distance < carveRadius) {
							cornerValues[cornerMap[cornerIndex + indexOffset]] = (isSubtractive) ? 0 : 1;
						}
					}
				}
				//DrawDebugPoint(GetWorld(), FVector3d(meshLocation.X + ((iY == 0) ? iX : boxSize - iX), meshLocation.Y + iY, meshLocation.Z + iZ) + offset, 5.0f, (cornerValues[cornerMap[cornerIndex + indexOffset]] == 1) ? FColor(0, 255, 0) : FColor(255, 0, 0), true, 20);
				triIndex += cornerValues[cornerMap[cornerIndex + indexOffset]] * powerOf2[cornerIndex];
				cornerCount += cornerValues[cornerMap[cornerIndex + indexOffset]];
				cornerIndex++;
			}
		}
	}
	cornerCounts[indexOffset / 8] = cornerCount;
	return triIndex;
}

/// <summary>
/// Calculate UV texture coordinates, projected from closest matching plane
/// </summary>
/// <param name="a">Vertex 0</param>
/// <param name="b">Vertex 1</param>
/// <param name="c">Vertex 2</param>
/// <param name="uvs">Array of length 3 to output UV vectors to</param>
void AMarchedCube::CalculateUVsAndNormal(FVector3d a, FVector3d b, FVector3d c, FVector2f* uvs, FVector3f* normal) {
	FVector3d s1 = b - a;
	FVector3d s2 = c - a;
	FVector3d norm = FVector3d::CrossProduct(s1, s2);
	norm.Normalize();
	*normal = -FVector3f(norm);
	norm.X = abs(norm.X);
	norm.Y = abs(norm.Y);
	norm.Z = abs(norm.Z);
	// Determine normals based on most relevant X/Y/Z plane
	if (norm.X >= norm.Z && norm.X >= norm.Y) {
		uvs[0] = FVector2f(a.Z, a.Y) / uvMax;
		uvs[1] = FVector2f(b.Z, b.Y) / uvMax;
		uvs[2] = FVector2f(c.Z, c.Y) / uvMax;
	}
	else if (norm.Z >= norm.X && norm.Z >= norm.Y) {
		uvs[0] = FVector2f(a.X, a.Y) / uvMax;
		uvs[1] = FVector2f(b.X, b.Y) / uvMax;
		uvs[2] = FVector2f(c.X, c.Y) / uvMax;
	}
	else if (norm.Y >= norm.X && norm.Y >= norm.Z) {
		uvs[0] = FVector2f(a.X, a.Z) / uvMax;
		uvs[1] = FVector2f(b.X, b.Z) / uvMax;
		uvs[2] = FVector2f(c.X, c.Z) / uvMax;
	}
}

/// <summary>
/// Gets cornerCounts value at index, or default of 0 if index out of bounds
/// </summary>
/// <param name="index">Index to try</param>
/// <returns></returns>
int AMarchedCube::TryCornerCount(int index) {
	if (index >= 0 && index < cornerCounts.Num()) {
		return cornerCounts[index];
	}
	else {
		return 0;
	}
}

/// <summary>
/// Initialises cornerMap to map each shared corner position to single value
/// </summary>
void AMarchedCube::InitialiseCorners() {
	int cornerValueIndex = 0;
	int cornerIndex = 0;
	int cornerTotal = 0;
	for (int Z = 0; Z < boxNum; Z++) {
		for (int Y = 0; Y < boxNum; Y++) {
			for (int X = 0; X < boxNum; X++) {
				cornerIndex = 0;
				// Single box
				for (int iZ = 0; iZ < 2 * boxSize; iZ += boxSize) {
					for (int iY = 0; iY < 2 * boxSize; iY += boxSize) {
						for (int iX = 0; iX < 2 * boxSize; iX += boxSize) {
							// Only add a cornerValue if the corner does not overlap a previously added corner
							if (Z > 0 && (cornerIndex == 0 || cornerIndex == 1 || cornerIndex == 2 || cornerIndex == 3)) {
								cornerMap.Add(cornerIndex + cornerTotal, cornerMap[cornerPairsZ[cornerIndex] + (cornerTotal - (8 * boxNum * boxNum))]);
							}
							else if (Y > 0 && (cornerIndex == 0 || cornerIndex == 1 || cornerIndex == 4 || cornerIndex == 5)) {
								cornerMap.Add(cornerIndex + cornerTotal, cornerMap[cornerPairsY[cornerIndex] + (cornerTotal - (8 * boxNum))]);
							}
							else if (X > 0 && (cornerIndex == 0 || cornerIndex == 3 || cornerIndex == 4 || cornerIndex == 7)) {
								cornerMap.Add(cornerIndex + cornerTotal, cornerMap[cornerPairsX[cornerIndex] + (cornerTotal - 8)]);
							}
							else {
								// First instance of a corner, add to cornerValues
								if (fillTop && Z == boxNum - 1 && iZ > 0) {
									cornerValues.Add(1 - defaultCornerValue);
								}
								else {
									cornerValues.Add(defaultCornerValue);
								}
								cornerMap.Add(cornerIndex + cornerTotal, cornerValueIndex);
								cornerValueIndex++;
							}
							cornerIndex++;
						}
					}
				}
				cornerTotal += 8;
			}
		}
	}
}
