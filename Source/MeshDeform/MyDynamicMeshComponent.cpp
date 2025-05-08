// Fill out your copyright notice in the Description page of Project Settings.


#include "MyDynamicMeshComponent.h"

UMyDynamicMeshComponent::UMyDynamicMeshComponent() {

}

void UMyDynamicMeshComponent::BeginPlay()
{
	EditMesh([&](FDynamicMesh3 mesh) {
		mesh.Clear();
		mesh.AppendVertex(FVector3d(0, 0, 0));
		mesh.AppendVertex(FVector3d(10, 0, 0));
		mesh.AppendVertex(FVector3d(10, 10, 0));
		mesh.AppendTriangle(0, 1, 2);
		mesh.AppendVertex(FVector3d(0, 0, 0));
		mesh.AppendVertex(FVector3d(10, 0, 0));
		mesh.AppendVertex(FVector3d(10, 10, 0));
		mesh.AppendTriangle(3, 4, 5);
		});
	NotifyMeshUpdated();
	
}
