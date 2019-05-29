// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshComponent.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Classes/Materials/MaterialInterface.h"
#include "Math/NumericLimits.h"
#include <random>
#include "MyStaticLibrary.generated.h"

class ATerrainTile;

/**
 * enum to differ tile borders
 */
UENUM(BlueprintType)
enum class ETileBorder : uint8
{
	ETB_Top UMETA(DisplayName = "Top Border"),
	ETB_Right UMETA(DisplayName = "Right Border"),
	ETB_Bottom UMETA(DisplayName = "Bottom Border"),
	ETB_Left UMETA(DisplayName = "Left Border"),
	ETB_Invalid UMETA(DisplayName = "Invalid")
};

/**
 * enum to differ what type of input device is used
 */
UENUM(BlueprintType)
enum class EControllerType : uint8
{
	ECT_Keyboard UMETA(DisplayName = "Keyboard"),
	ECT_XBox UMETA(DisplayName = "XBox"),
	ECT_None UMETA(DisplayName = "None")
};

/**
 * struct that defines an integer vector in 2D space, since Unreal decides to not come up with such a thing by default
 */
USTRUCT(BlueprintType)
struct FIntVector2D
{
	GENERATED_USTRUCT_BODY()

	int32 X;
	int32 Y;

	FIntVector2D(int32 x, int32 y)
	{
		X = x;
		Y = y;
	}

	FIntVector2D()
	{
		X = 0;
		Y = 0;
	}

	FORCEINLINE bool operator==(const FIntVector2D& Other) const
	{
		return (X == Other.X) && (Y == Other.Y);
	}

	FORCEINLINE bool operator!=(const FIntVector2D& Other) const
	{
		return (X != Other.X) || (Y != Other.Y);
	}

	FIntVector2D operator-(const FIntVector2D& Other) const
	{
		return FIntVector2D(X - Other.X, Y - Other.Y);
	}

	FIntVector2D operator+(const FIntVector2D& Other) const
	{
		return FIntVector2D(X + Other.X, Y + Other.Y);
	}

	FIntVector2D operator*(const FIntVector2D& Other) const
	{
		return FIntVector2D(X * Other.X, Y * Other.Y);
	}

	FIntVector2D operator*(const int32& Other) const
	{
		return FIntVector2D(X * Other, Y * Other);
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("X=%i Y=%i"), X, Y);
	}

	friend FORCEINLINE uint32 GetTypeHash(const FIntVector2D& Vector)
	{
		// Note: this assumes there's no padding in FINTVector2D that could contain uncompared data.
		//return FCrc::MemCrc_DEPRECATED(&Vector, sizeof(Vector));
		//return FCrc::MemCrc32(&Vector, sizeof(FIntVector2D));
		return FCrc::MemCrc_DEPRECATED(&Vector, sizeof(FIntVector2D));
	}
};

/**
 * struct that contains a checkpoint spawn job
 */
USTRUCT()
struct FCheckpointSpawnJob
{
	GENERATED_USTRUCT_BODY()

	// the checkpoint's ID
	int32 CheckpointID = -1;

	// the checkpoint's transform
	UPROPERTY()
	FTransform CheckpointTransform = FTransform();

	// the sector the checkpoint should be created for
	UPROPERTY()
	FIntVector2D CheckpointSector = FIntVector2D();

	/**
	 * @DEPRECATED, use other constructor
	 */
	FCheckpointSpawnJob()
	{

	}

	FCheckpointSpawnJob(const uint32 ID, const FTransform& Transform, const FIntVector2D Sector)
	{
		CheckpointID = ID;
		CheckpointTransform = Transform;
		CheckpointSector = Sector;
	}

};

/**
 * struct that is used to store track information for each sector
 */
USTRUCT()
struct FSectorTrackInfo
{
	GENERATED_USTRUCT_BODY()

	// does the sector contains a track?
	bool bSectorHasTrack = false;

	// start point of the track in this sector
	UPROPERTY()
	FVector2D TrackEntryPoint = FVector2D();

	// end point of the track in this sector
	UPROPERTY()
	FVector2D TrackExitPoint = FVector2D();

	// previous track sector
	UPROPERTY()
	FIntVector2D PreviousTrackSector = FIntVector2D();

	// following track sector
	UPROPERTY()
	FIntVector2D FollowingTrackSector = FIntVector2D();

	// elevation of the track exit point
	UPROPERTY()
	float TrackExitPointElevation = 0.f;

	// first bézier control point
	UPROPERTY()
	FVector FirstBezierControlPoint = FVector();

	// second bézier control point
	UPROPERTY()
	FVector SecondBezierControlPoint = FVector();

	// position of Y0 for the track exit point
	UPROPERTY()
	FVector Y0Position = FVector();

	// position of Y1 for the track exit point
	UPROPERTY()
	FVector Y1Position = FVector();

	// points sampled from the bezier cuve given by entry/exit points and control points
	UPROPERTY()
	TArray<FVector> PointsOnBezierCurve;

	/**
	 * The ID of the checkpoint in this sector
	 * if no track exists, there is no track and the ID is -1
	 */
	UPROPERTY()
	int32 CheckpointID = -1;

	// sector has no track
	FSectorTrackInfo()
	{

	}

	// sector has a track
	FSectorTrackInfo(FVector2D EntryPoint, FVector2D ExitPoint, FIntVector2D PreviousSector, FIntVector2D FollowingSector)
	{
		bSectorHasTrack = true;
		TrackEntryPoint = EntryPoint;
		TrackExitPoint = ExitPoint;
		PreviousTrackSector = PreviousSector;
		FollowingTrackSector = FollowingSector;
	}
};

/**
 * struct to store a track segment and perform calculations on it
 */
USTRUCT()
struct FTrackSegment
{
	GENERATED_USTRUCT_BODY()

	// points that define the track segment
	UPROPERTY()
	TArray<FVector2D> DefiningPoints;

	// the middle point of the track at the start of the segment
	UPROPERTY()
	FVector TrackStartMiddlePoint;

	// the middle point of the track at the end of the segment
	UPROPERTY()
	FVector TrackEndMiddlePoint;

	// values for the bounding rectangle of the above points
	float MinX = TNumericLimits<float>::Max();
	float MaxX = TNumericLimits<float>::Min();
	float MinY = TNumericLimits<float>::Max();
	float MaxY = TNumericLimits<float>::Min();

	/**
	 * minimum parameters such that Points with X in [i_min * UnitSize, i_max * UnitSize] and Y in [j_min * UnitSize, j_max * UnitSize] lie within rectanglular bounding box
	 */
	int32 i_min = 0;
	int32 i_max = 0;
	int32 j_min = 0;
	int32 j_max = 0;

	/**
	 * size between to adjacent terrain mesh vertices
	 */
	float UnitSize = 0.f;

	/**
	 * array of points that lie on the track segment (or its rectangular bounding box)
	 */
	UPROPERTY()
	TArray<FVector> PointsOnTrackSegment;

	/**
	 * the height of the base line of the track segment
	 * i.e. height of defining points 1 and 2
	 */
	float BaseLineHeight = 0.f;

	/**
	 * the height of the end line of the track segment
	 * i.e. height of defining points 3 and 4
	 */
	float EndLineHeight = 0.f;

	/**
	 * size of a terrain tile edge
	 */
	float TileEdgeSize = 0.f;

	/**
	 * error tolerance when calculating if a point lies between the defining points
	 */
	float ErrorTolerance = 0.f;

	/**
	 * offset between track and terrain, used in elevation interpolation
	 */
	float ElevationOffset = 0.f;

	bool bIsLastSegmentOnTrack = false;
	bool bIsFirstSegmentOnTrack = false;

	/**
	 * @DEPRECATED, use other constructor!
	 */
	FTrackSegment()
	{
		for (int32 i = 0; i < 4; ++i)
		{
			DefiningPoints.Add(FVector2D());
		}
		BaseLineHeight = 0.f;
		EndLineHeight = 0.f;
		UE_LOG(LogTemp, Error, TEXT("Using wrong FTrackSegment constructor"));
	}

	FTrackSegment(const FVector Point1, const FVector Point2, const FVector Point3, const FVector Point4, const FVector TrackMiddlePointStart, const FVector TrackMiddlePointEnd, const float TileEdgeSize, const float IncludeErrorTolerance, const float TrackElevationOffset)
	{
		DefiningPoints.Add(FVector2D(Point1.X, Point1.Y));
		DefiningPoints.Add(FVector2D(Point2.X, Point2.Y));
		DefiningPoints.Add(FVector2D(Point3.X, Point3.Y));
		DefiningPoints.Add(FVector2D(Point4.X, Point4.Y));

		TrackStartMiddlePoint = TrackMiddlePointStart;
		TrackEndMiddlePoint = TrackMiddlePointEnd;

		BaseLineHeight = TrackMiddlePointStart.Z;
		EndLineHeight = TrackMiddlePointEnd.Z;

		this->TileEdgeSize = TileEdgeSize;
		this->ErrorTolerance = IncludeErrorTolerance;
		ElevationOffset = TrackElevationOffset;
	}

	/**
	 * calculates all points that lie on the given track segment
	 * @param UnitSize The distance between to adjacent mesh vertices
	 * @param UseTightBoundingBox Using a tight bounding box will return points that lie in the initially defined track segment, false if all points in the rectangular bounding box around the given track segment should be returned
	 * @param OUTPointsOnTrack All points that lie on the track as specified by UseTightBoundingBox with correct height
	 */
	void CalculatePointsOnTrack(const float UnitSize, const bool UseTightBoundingBox, TArray<FVector>& OUTPointsOnTrack, const bool IsFirstSegmentOnTrack, const bool IsLastSegmentOnTrack)
	{
		this->UnitSize = UnitSize;

		CalculateBoundingRectangle();

		bIsFirstSegmentOnTrack = IsFirstSegmentOnTrack;
		bIsLastSegmentOnTrack = IsLastSegmentOnTrack;

		if (bIsFirstSegmentOnTrack || bIsLastSegmentOnTrack)
		{
			UE_LOG(LogTemp, Error, TEXT("Segment is %s segment on track"), bIsFirstSegmentOnTrack ? TEXT("first") : TEXT("last"));
		}

		CalculatePointsInBoundingBox(UseTightBoundingBox);
		OUTPointsOnTrack = PointsOnTrackSegment;

	}

	/**
	 * ! For internal use only, use CalculatePointsOnTrack instead !
	 */
	void CalculateBoundingRectangle()
	{
		for (const FVector2D Point : DefiningPoints)
		{
			MinX = Point.X < MinX ? Point.X : MinX;
			MaxX = Point.X > MaxX ? Point.X : MaxX;
			MinY = Point.Y < MinY ? Point.Y : MinY;
			MaxY = Point.Y > MaxY ? Point.Y : MaxY;
		}

	}


	/**
	 * ! For internal use only, use CalculatePointsOnTrack instead !
	 */
	void CalculatePointsInBoundingBox(const bool UseTightBoundingBox)
	{
		j_min = FMath::FloorToInt(MinY / UnitSize);
		j_max = FMath::CeilToInt(MaxY / UnitSize);
		i_min = FMath::FloorToInt(MinX / UnitSize);
		i_max = FMath::CeilToInt(MaxX / UnitSize);

		// calculate points in rectangular bounding box
		for (int32 i = i_min; i <= i_max; ++i)
		{
			for (int32 j = j_min; j <= j_max; ++j)
			{
				const FVector2D Point = FVector2D(i * UnitSize, j * UnitSize);

				PointsOnTrackSegment.Add(FVector(Point.X, Point.Y, 0.f/*InterpolatePointElevationTriangle(Point, PointA, PointB, PointC)*/ /*InterpolatePointElevation(Point)*/));
			}
		}

		/**
		 * filter out points that probably lie within the next or previous track segment, as they will be processed there
		 * don't do this for the last track segment in the tile, since there is no next track segment
		 *
		 *				X3--------------------X2
		 *				|					  |
		 *				|					  |
		 *				|					  |
		 *				X0--------------------X1
		 */

		FVector2D X2X3 = DefiningPoints[3] - DefiningPoints[2];
		FVector2D X1X0 = DefiningPoints[0] - DefiningPoints[1];
		FVector2D X0X3 = DefiningPoints[3] - DefiningPoints[0];
		FVector2D X1X2 = DefiningPoints[2] - DefiningPoints[1];

		for (int32 i = PointsOnTrackSegment.Num() - 1; i >= 0; --i)
		{
			FVector2D Pt = FVector2D(PointsOnTrackSegment[i].X, PointsOnTrackSegment[i].Y);

			/**
			 * filter out points that are within our bounding rectangle, but not within the track segment's start and end line (these points get handled by the next / previous track segment)
			 * point lies between X1X0 and X2X3 if det(X2X3, X2Pt) (== DetA) and det(X1X0, X1Pt) (== DetB) have different signs
			 */
			float DetA = FVector2D::CrossProduct(X2X3, Pt - DefiningPoints[2]);
			float DetB = FVector2D::CrossProduct(X1X0, Pt - DefiningPoints[1]);

			// since sometimes the determinant of a point lying on the same line as the defining points is not 0 but somewhere close to it, an error tolerance is introduced 
			if ((DetA > ErrorTolerance && DetB > ErrorTolerance) || (DetA < -ErrorTolerance && DetB < -ErrorTolerance))
			{
				PointsOnTrackSegment.RemoveAt(i);
				continue;
			}

			/**
			 * this section should help fix the terrain discontinuity on borders if one tile defines a track constraint that the other tile does not define
			 */

			// only continue if current point lies on one of the tile's borders
			if (FMath::IsNearlyZero(Pt.X) || FMath::IsNearlyEqual(Pt.X, TileEdgeSize) || FMath::IsNearlyZero(Pt.Y) || FMath::IsNearlyEqual(Pt.Y, TileEdgeSize))
			{
				// only continue if point is outside of track borders (X0X3 && X1X2)
				float DetC = FVector2D::CrossProduct(X0X3, Pt - DefiningPoints[0]);
				float DetD = FVector2D::CrossProduct(X1X2, Pt - DefiningPoints[1]);

				if ((DetC > ErrorTolerance && DetD > ErrorTolerance) || (DetC < -ErrorTolerance && DetD < -ErrorTolerance))
				{
					float MinDistance = 0.f;

					if (bIsFirstSegmentOnTrack || bIsLastSegmentOnTrack)
					{
						// X1X0 (or X2X3) will lie on border
						int32 Index1 = -1;
						int32 Index2 = -1;
						if (bIsFirstSegmentOnTrack)
						{
							Index1 = 0;
							Index2 = 1;
						}
						if (bIsLastSegmentOnTrack)
						{
							Index1 = 3;
							Index2 = 2;
						}
						// calculate minimum distance between point and line segment
						MinDistance = FMath::Min<float>
							(
								FVector2D::Distance(Pt, DefiningPoints[Index1]),
								FVector2D::Distance(Pt, DefiningPoints[Index2])
							);

					}
					else
					{
						MinDistance = FMath::Min3<float>
							(
								FVector2D::Distance(Pt, DefiningPoints[0]),
								FVector2D::Distance(Pt, DefiningPoints[1]),
								FVector2D::Distance(Pt, DefiningPoints[2])
							);
						MinDistance = FMath::Min<float>
							(
								MinDistance,
								FVector2D::Distance(Pt, DefiningPoints[3])
							);
					}

					// if minimum distance is greater than UnitSize, we know there is at least one other point between the current point and the defining point, so we can remove the current point from the constraints
					if (MinDistance > UnitSize)
					{
						PointsOnTrackSegment.RemoveAt(i);
						continue;
					}
				}
			}
		}

		// for every remaining point, calculate the points elevation by interpolating the height of the triangle it is in
		for (FVector& Point : PointsOnTrackSegment)
		{
			// get triangle the point lies in
			FVector PointA, PointB, PointC;
			FVector2D Point2D = FVector2D(Point.X, Point.Y);

			if (bIsFirstSegmentOnTrack || bIsLastSegmentOnTrack)
			{
				UE_LOG(LogTemp, Warning, TEXT("Point %s"), *Point2D.ToString());
			}

			DetermineTrianglePointLiesIn(Point2D, PointA, PointB, PointC);

			Point.Z = InterpolatePointElevationTriangle(Point2D, PointA, PointB, PointC);
		}

		return;

		// this yields bad results because the mesh resolution isn't high enough
		//if (!UseTightBoundingBox) { return; }
		//// use cross product (in 2D space determinant) to check if point lies within original track segment
		//// point numeration switches from 1-based to 0-based to better comply with array indices
		//FVector2D Vector_Point0Point1 = DefiningPoints[1] - DefiningPoints[0];
		//FVector2D Vector_Point1Point2 = DefiningPoints[2] - DefiningPoints[1];
		//FVector2D Vector_Point2Point3 = DefiningPoints[3] - DefiningPoints[2];
		//FVector2D Vector_Point3Point0 = DefiningPoints[0] - DefiningPoints[3];

		//// iterate all points and remove those that don't lie within original track segment
		//for (int k = PointsOnTrackSegment.Num() - 1; k >= 0; --k)
		//{
		//	FVector2D Vector_Point0P = FVector2D(PointsOnTrackSegment[k].X, PointsOnTrackSegment[k].Y) - DefiningPoints[0];
		//	FVector2D Vector_Point1P = FVector2D(PointsOnTrackSegment[k].X, PointsOnTrackSegment[k].Y) - DefiningPoints[1];
		//	FVector2D Vector_Point2P = FVector2D(PointsOnTrackSegment[k].X, PointsOnTrackSegment[k].Y) - DefiningPoints[2];
		//	FVector2D Vector_Point3P = FVector2D(PointsOnTrackSegment[k].X, PointsOnTrackSegment[k].Y) - DefiningPoints[3];

		//	float detA = FVector2D::CrossProduct(Vector_Point0Point1, Vector_Point0P);
		//	float detB = FVector2D::CrossProduct(Vector_Point1Point2, Vector_Point1P);
		//	float detC = FVector2D::CrossProduct(Vector_Point2Point3, Vector_Point2P);
		//	float detD = FVector2D::CrossProduct(Vector_Point3Point0, Vector_Point3P);

		//	// point lies within convex quad, if all cross poducts have the same sign
		//	if (!((detA <= 0 && detB <= 0 && detC <= 0 && detD <= 0) || (detA >= 0 && detB >= 0 && detC >= 0 && detD >= 0)))
		//	{
		//		PointsOnTrackSegment.RemoveAt(k);
		//	}
		//}
	}

	/**
	 * returns the three points of the triangle the given point lies in
	 * if the given point does not lie within a triangle, it get's projected into the nearest one and OUTPoint contains the projected point's X and Y coordinates
	 * @param OUTPoint The point for which the triangle should be determined
	 * @param OUTPointA First point of the triangle the point lies inside
	 * @param OUTPointB Second point of the triangle the point lies inside
	 * @param OUTPointC Third point of the triangle the point lies inside
	 */
	void DetermineTrianglePointLiesIn(FVector2D& OUTPoint, FVector& OUTPointA, FVector& OUTPointB, FVector& OUTPointC)
	{
		FVector2D X0 = DefiningPoints[0];
		FVector2D X1 = DefiningPoints[1];
		FVector2D X2 = DefiningPoints[2];
		FVector2D X3 = DefiningPoints[3];
		FVector2D T0 = FVector2D(TrackStartMiddlePoint.X, TrackStartMiddlePoint.Y);
		FVector2D T1 = FVector2D(TrackEndMiddlePoint.X, TrackEndMiddlePoint.Y);

		/**
		 *			X3----------T1----------X2
		 *			| \			| \			|
		 *			|   \		|   \		|
		 *			|	  \		|     \		|
		 *			|	    \	|		\	|
		 *			|	      \ |		  \	|
		 *			X0----------T0----------X1
		 *
		 */

		// first check the two inmost triangles [ (T0 T1 X3) and (T0 X1 T1) ]
		if (CheckPointInsideTriangle(OUTPoint, T0, T1, X3))
		{
			if (bIsLastSegmentOnTrack || bIsFirstSegmentOnTrack)
				UE_LOG(LogTemp, Warning, TEXT("lies in triangle (T0 T1 X3)"));
			OUTPointA = FVector(T0, BaseLineHeight);
			OUTPointB = FVector(T1, EndLineHeight);
			OUTPointC = FVector(X3, EndLineHeight);
			return;
		}
		if (CheckPointInsideTriangle(OUTPoint, T0, X1, T1))
		{
			if (bIsLastSegmentOnTrack || bIsFirstSegmentOnTrack)
				UE_LOG(LogTemp, Warning, TEXT("lies in triangle (T0 X1 T1)"));
			OUTPointA = FVector(T0, BaseLineHeight);
			OUTPointB = FVector(X1, BaseLineHeight);
			OUTPointC = FVector(T1, EndLineHeight);
			return;
		}
		
		// the two outmost triangles
		if (CheckPointInsideTriangle(OUTPoint, X0, T0, X3))
		{
			if (bIsLastSegmentOnTrack || bIsFirstSegmentOnTrack)
				UE_LOG(LogTemp, Warning, TEXT("lies in triangle (X0 T0 X3)"));
			OUTPointA = FVector(X0, BaseLineHeight);
			OUTPointB = FVector(T0, BaseLineHeight);
			OUTPointC = FVector(X3, EndLineHeight);
			return;
		}
		if (CheckPointInsideTriangle(OUTPoint, X1, X2, T1))
		{
			if (bIsLastSegmentOnTrack || bIsFirstSegmentOnTrack)
				UE_LOG(LogTemp, Warning, TEXT("lies in triangle (X1 X2 T1)"));
			OUTPointA = FVector(X1, BaseLineHeight);
			OUTPointB = FVector(X2, EndLineHeight);
			OUTPointC = FVector(T1, EndLineHeight);
			return;
		}
		else
		{
			// point lies outside of a triangle -> project point onto X0X3 and X1X2 and use that projection, whose distance to the original point is smallest

			// don't project point if it lies on one of the tile's borders (this would lead to erroneous elevation values)
			if ((bIsFirstSegmentOnTrack || bIsLastSegmentOnTrack) && CheckPointLiesOnTileBorder(OUTPoint))
			{
				// use the elevation of the nearest track point(X0 X1 X2 X3) instead
				TArray<FVector2D> TrackPoints;
				TrackPoints.Add(X0);
				TrackPoints.Add(X1);
				TrackPoints.Add(X2);
				TrackPoints.Add(X3);
				OUTPoint = GetClosestPoint(OUTPoint, TrackPoints);
				if (!TrackPoints.Contains(X0) || !TrackPoints.Contains(X1))
				{
					OUTPointA = FVector(OUTPoint, BaseLineHeight);
				}
				else
				{
					OUTPointA = FVector(OUTPoint, EndLineHeight);
				}
				OUTPointB = FVector(TrackPoints[0], 0.f);
				OUTPointC = FVector(TrackPoints[1], 0.f);
				return;
			}
			else
			{
				FVector2D Projection_Point_X0X3 = ProjectPointOnLineSegment(OUTPoint, X0, X3);
				FVector2D Projection_Point_X1X2 = ProjectPointOnLineSegment(OUTPoint, X1, X2);

				if (FVector2D::Distance(OUTPoint, Projection_Point_X0X3) <= FVector2D::Distance(OUTPoint, Projection_Point_X1X2))
				{
					if (bIsLastSegmentOnTrack || bIsFirstSegmentOnTrack)
						UE_LOG(LogTemp, Warning, TEXT("does NOT lie inside a triangle. Projecting into (X0 T0 X3)"));
					OUTPoint = Projection_Point_X0X3;
					OUTPointA = FVector(X0, BaseLineHeight);
					OUTPointB = FVector(T0, BaseLineHeight);
					OUTPointC = FVector(X3, EndLineHeight);
					return;
				}
				else
				{
					if (bIsLastSegmentOnTrack || bIsFirstSegmentOnTrack)
						UE_LOG(LogTemp, Warning, TEXT("does NOT lie inside a triangle. Projecting into (X1 X2 T1)"));
					OUTPoint = Projection_Point_X1X2;
					OUTPointA = FVector(X1, BaseLineHeight);
					OUTPointB = FVector(X2, EndLineHeight);
					OUTPointC = FVector(T1, EndLineHeight);
					return;
				}
			}
		}

	}

	/**
	 * checks which of the provided points is closest to the given point
	 * after execution OUTPointsToCheck does not contain the closest point anymore
	 * @param OriginalPoint The base point to calculate distances for
	 * @param OUTPointsToCheck Array containing all points to calculate distances to
	 * @return Point that is closest to given point
	 */
	FVector2D GetClosestPoint(const FVector2D OriginalPoint, TArray<FVector2D>& OUTPointsToCheck)
	{
		FVector2D CurrentClosestPoint;
		float CurrentMinimumDistance = TNumericLimits<float>::Max();
		for (const FVector2D Point : OUTPointsToCheck)
		{
			float Distance = FVector2D::Distance(OriginalPoint, Point);
			if (Distance < CurrentMinimumDistance)
			{
				CurrentMinimumDistance = Distance;
				CurrentClosestPoint = Point;
			}
		}
		OUTPointsToCheck.Remove(CurrentClosestPoint);
		return CurrentClosestPoint;
	}

	/**
	 * checks if the given point lies on a tile border
	 * @param Point The point to check
	 * @return True if the point lies on a border of the tile, false otherwise
	 */
	bool CheckPointLiesOnTileBorder(const FVector2D Point)
	{
		if (FMath::IsNearlyZero(Point.X) || FMath::IsNearlyZero(Point.Y) || FMath::IsNearlyEqual(Point.X, TileEdgeSize) || FMath::IsNearlyEqual(Point.Y, TileEdgeSize))
		{
			return true;
		}
		else
		{
			return false;
		}
	}


	/**
	 * checks whether the given point lies within the triangle given by the three points
	 * @param Point The point to check
	 * @param A Triangle point 1
	 * @param B Triangle point 2
	 * @param C Triangle point 3
	 * @return True if the point lies within the triangle, false otherwise
	 */
	bool CheckPointInsideTriangle(const FVector2D Point, const FVector2D A, const FVector2D B, const FVector2D C)
	{
		FVector2D AB = B - A;
		FVector2D BC = C - B;
		FVector2D CA = A - C;

		FVector2D APoint = Point - A;
		FVector2D BPoint = Point - B;
		FVector2D CPoint = Point - C;

		double detA = FVector2D::CrossProduct(AB, APoint);
		double detB = FVector2D::CrossProduct(BC, BPoint);
		double detC = FVector2D::CrossProduct(CA, CPoint);

		if ((detA <= 0 && detB <= 0 && detC <= 0) || (detA >= 0 && detB >= 0 && detC >= 0))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	/**
	 * ! For internal use only !
	 * linearly interpolates the given points elevation
	 */
	float InterpolatePointElevation(const FVector2D Point)
	{
		// calculate distance from point to base line (not segment, because points outside the track need to get the same elevation as points inside the track
		float DistanceToBaseLine = CalculateMinimumDistancePointLine(Point, DefiningPoints[0], DefiningPoints[1]);

		// calculate distance from point to end line
		float DistanceToEndLine = CalculateMinimumDistancePointLine(Point, DefiningPoints[3], DefiningPoints[2]);
		if (DistanceToBaseLine + DistanceToEndLine == 0.f) { return 0.f; }

		float Alpha = DistanceToBaseLine / (DistanceToBaseLine + DistanceToEndLine);

		return (FMath::Lerp<float, float>(BaseLineHeight, EndLineHeight, Alpha) - ElevationOffset);
	}

	FVector2D ProjectPointOnLineSegment(const FVector2D Point, const FVector2D LineStartPoint, const FVector2D LineEndPoint)
	{
		const float L2 = FVector2D::DistSquared(LineStartPoint, LineEndPoint);
		if (FMath::IsNearlyZero(L2))
		{
			return LineStartPoint;
		}
		const float t = FMath::Max<float>(0.f, FMath::Min<float>(1.f, FVector2D::DotProduct(Point - LineStartPoint, LineEndPoint - LineStartPoint) / L2));

		return (LineStartPoint + t * (LineEndPoint - LineStartPoint));
	}


	float InterpolatePointElevationTriangle(const FVector2D Point, const FVector PointA, const FVector PointB, const FVector PointC)
	{
		// barycentric point weights
		double W_A = 0.;
		double W_B = 0.;
		double W_C = 0.;

		double Denominator = (PointB.Y - PointC.Y) * (PointA.X - PointC.X) + (PointC.X - PointB.X) * (PointA.Y - PointC.Y);

		W_A = ((PointB.Y - PointC.Y) * (Point.X - PointC.X) + (PointC.X - PointB.X) * (Point.Y - PointC.Y)) / Denominator;

		W_B = ((PointC.Y - PointA.Y) * (Point.X - PointC.X) + (PointA.X - PointC.X) * (Point.Y - PointC.Y)) / Denominator;

		W_C = 1 - W_A - W_B;

		return (W_A * PointA.Z + W_B * PointB.Z + W_C * PointC.Z) - ElevationOffset;

	}

	/**
	 * duplicated from UMyStaticLibrary so it can be used in this struct
	 */
	float CalculateMinimumDistancePointLineSegment(const FVector2D Point, const FVector2D LineStartPoint, const FVector2D LineEndPoint)
	{
		const float L2 = FVector2D::DistSquared(LineStartPoint, LineEndPoint);
		if (FMath::IsNearlyZero(L2))
		{
			return FVector2D::Distance(Point, LineStartPoint);
		}
		const float t = FMath::Max<float>(0.f, FMath::Min<float>(1.f, FVector2D::DotProduct(Point - LineStartPoint, LineEndPoint - LineStartPoint) / L2));

		const FVector2D Projection = LineStartPoint + t * (LineEndPoint - LineStartPoint);
		return FVector2D::Distance(Point, Projection);
	}

	/**
	 * calculates the minimal distance of a point to a line
	 * equation taken from https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_an_equation
	 * @param Point The point for which the distance to a line should be calculated
	 * @param LinePoint1 First point that lies on the line
	 * @param LinePoint2 Second point that lies on the line
	 * @return The minimum distance between the point and the line
	 */
	float CalculateMinimumDistancePointLine(const FVector2D Point, const FVector2D LinePoint1, const FVector2D LinePoint2)
	{
		float Numerator = FMath::Abs<float>
			(
				(LinePoint2.Y - LinePoint1.Y) * Point.X - (LinePoint2.X - LinePoint1.X) * Point.Y +
				LinePoint2.X * LinePoint1.Y - LinePoint2.Y * LinePoint1.X
			);

		float Denominator = FMath::Sqrt
			(
				FMath::Pow(LinePoint2.Y - LinePoint1.Y, 2) +
				FMath::Pow(LinePoint2.X - LinePoint1.X, 2)
			);
		if (Denominator == 0.f)
		{
			UE_LOG(LogTemp, Error, TEXT("Division by 0 in CalculateMinimumDistancePointLine in FTrackSegment"));
			return 0.f;
		}
		return Numerator / Denominator;
	}
};

/**
 * struct for fractal noise terrain generation settings
 * to be used as subcategory in FTerrainSettings
 * all parameters are used as suggested in "Terrain Modeling: A Constrained Fractal Model" by Farès Belhadj in 2007
 */
USTRUCT(BlueprintType)
struct FFractalNoiseTerrainSettings
{
	GENERATED_USTRUCT_BODY()

	/**
	 * used to translate random value interval [-1, 1] in random displacement calculation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float rt = 0.0f;

	/**
	 * scale factor to scale random value in random displacement calculation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float rs = -0.5f;

	/**
	 * space dimension used in random displacement calculation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float n = 1.f;

	/**
	 * approximation of Hurst's parameter
	 * used to control fractal dimension in random displacement calculation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float H = 0.5f;

	/**
	 * used to tune the non-linear interpolation curve in the midpoint displacement process
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float I = -0.5;

	/**
	 * used to tune the non-linear interpolation curve in the midpoint displacement bottom-up process
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float I_bu = 0.5f;

	/**
	 * number of iterations for the triangle edge algorithms
	 * reducing this number reduces terrain quality but greatly increases performance
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TriangleEdgeIterations = 6;
};

/**
 * struct with all settings to configure the track generation
 */
USTRUCT(BlueprintType)
struct FTrackGenerationSettings
{
	GENERATED_USTRUCT_BODY()

	/**
	 * defines how many points on the Bézier curve should be calculated
	 * a higher number creates a smoother mesh, but also increases performance costs
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrackResolution = 20;

	/**
	 * the overall track width in cm
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackWidth = 1500;

	/**
	 * if enabled, tries to only add those points as track constraints that lie on a track segment, otherwise all points in a rectangle around the track segment get added as constraints
	 * disable to save performance
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseTightTrackBoundingBox = false;

	/**
	 * parameter that controls the maximum elevation difference (in cm) between the track's starting and end point in one tile
	 * multiplied with result of normal distribution to obtain final elevation of track exit point in relation to track entry point
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaximumElevationDifference = 20000.f;

	/**
	 * the default elevation for the track's entry point
	 * used when the track for the very first sector is calculated (since there is no previous track with an exit point where we could get the elevation from)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultEntryPointHeight = 2000.f;

	/**
	 * the height in cm the terrain mesh should lie beneath the track mesh
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackElevationOffset = 10.f;

	/**
	 * The mean value of the normal distribution for calculating the second bezier control point.
	 * Defaults to 0.f
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CURVINESS_MEAN = 0.0f;

	/**
	 * the curviness of the track for the displacement calculation, will be used as standard deviation in a normal distribution with mean CURVINESS_MEAN
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurvinessDisplacement = 0.2f;

	/**
	 * the curviness of the track for the rotation calculation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurvinessRotation = 0.4f;

	/**
	 * The maximum angle the second bezier control point should be rotated by
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaximumRotationAngle = 45.f;

	/**
	 * quality of being hilly, used as standard deviation in a normal distribution with the average elevation between track entry and exit point as mean
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Hilliness = 2000.f;

	/**
	 * mean value for elevation difference between track entry and exit point
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Steepness_Mean = 0.f;

	/**
	 * standard deviation for elevation difference between track entry and exit point
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Steepness_Deviation = 0.2f;

	/**
	 * error tolerance for calculation if a point is inside the quad defined by a track segment's start and end line 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointInsideErrorTolerance = 100.f;
};


/**
 * struct for terrain settings
 */
USTRUCT(BlueprintType)
struct FTerrainSettings
{
	GENERATED_USTRUCT_BODY()

	/**
	 * Spawn elevation of the player above the track (in cm)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayerSpawnElevationOffset = 100.f;

	/**
	 * Track segment to spawn player at (0-based)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	int32 TrackSegmentToSpawnPlayerAt = 0;

	/**
	 * Offset for track segment to spawn player at
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackSegmentToSpawnPlayerAtOffset = 200.f;

	/**
	 * interpolation speed for the transition of the default pawn to the Hovercraft's location after all tiles are created
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionInterpolationSpeed = 10.f;

	/**
	 * offset above the Hovercraft's location where the transition should stop (in cm)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionElevationOffset = 5000.f;

	/**
	 * delta between target position and current position at which transition should stop (in cm)
	 * used because the interpolation needs a lot of time to actually reach the target position
	 * when using 0.f, expect to wait a long time before the pawn is switched
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionDeltaToStop = 100.f;

	/**
	 * to be removed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Point1Elevation = 0.f;

	/**
	* to be removed
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Point2Elevation = 0.f;

	/**
	* to be removed
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Point3Elevation = 0.f;

	/**
	* to be removed
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Point4Elevation = 0.f;

	/**
	* to be removed
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Point5Elevation = 0.f;

	/**
	 * size of a terrain tile in X direction in tile units
	 * @DEPRECATED
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TileSizeXUnits = 0;

	/**
	 * size of a terrain tile in Y direction in tile units
	 * @DEPRECATED
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TileSizeYUnits = 0;

	/**
	 * size of a terrain tile unit
	 * @DEPRECATED
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitTileSize = 0;

	// the edge length of a quadratic tile in cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TileEdgeSize = 65536;

	// shall async collision cooking be used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseAsyncCollisionCooking = true;

	// fractal noise terrain generation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFractalNoiseTerrainSettings FractalNoiseTerrainSettings;

	// track generation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTrackGenerationSettings TrackGenerationSettings;

	// height in cm at which a transition from low terrain material to medium terrain material should occur
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TerrainMaterialTransitionLowMediumElevation = -3000.f;

	// height in cm at which a transition from medium terrain material to high terrain material should occur
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TerrainMaterialTransitionMediumHighElevation = 80000.f;

	// variation of the transition elevation medium high parameter (in cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionElevationVariationMediumHigh = 2000.f;

	// variation of the transition elevation low medium parameter (in cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionElevationVariationLowMedium = 500.f;

	/** 
	 * array of materials that should be applied to the terrain
	 * index 0 is the default track material
	 * index 1 is the default terrain material for all elevations <= TerrainMaterialTransitionLowMediumElevation
	 * index 2 is the default terrain material for all elevations > TerrainMaterialTransitionLowMediumHeight and <= TerrainMaterialTransitionMediumHighHeight
	 * index 3 is the default terrain material for all elevations > TerrainMaterialTransitionMediumHighHeight
	 * corresponds to the mesh index in FTerrainJob
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UMaterialInterface*> Materials;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//UMaterialInterface* TerrainMaterial = nullptr;

	//// material that should be applied to the track
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//UMaterialInterface* TrackMaterial = nullptr;

	/**
	 * specifies the number of tiles that should be created around a tracked actor
	 * since it is the radius, a value of n will result in (2*n + 1) * (2*n + 1) tiles created around each tracked actor
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TilesToBeCreatedAroundActorRadius = 3;

	//Time in seconds after which a freed tile should be deleted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SecondsUntilFreeTileGetsDeleted = 30.f;

	// number of threads that should be used for terrain generation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 NumberOfThreadsToUse = 1;

	// how many meshes should be updated per frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MeshUpdatesPerFrame = 8;


};

/**
 * struct for vertex and triangle buffer
 */
USTRUCT()
struct FMeshData
{
	GENERATED_USTRUCT_BODY()

	// vertex buffer
	TArray<FRuntimeMeshVertexSimple> VertexBuffer;

	// triangle buffer
	UPROPERTY()
	TArray<int32> TriangleBuffer;

};

/**
 * struct for a job in which terrain is generated
 */
USTRUCT(BlueprintType)
struct FTerrainJob
{
	GENERATED_USTRUCT_BODY()

	// the terrain tile this job calculates mesh data for
	UPROPERTY()
	ATerrainTile* TerrainTile = nullptr;

	//// the terrain mesh data
	//UPROPERTY()
	//FMeshData TerrainMeshData;

	//// the track mesh data
	//UPROPERTY()
	//FMeshData TrackMeshData;

	/**
	* array where each index corresponds to one mesh section
	* first index corresponds to terrain
	* second index corresponds to track
	*/
	UPROPERTY()
	TArray<FMeshData> MeshData;

	FTerrainJob()
	{
		MeshData.Init(FMeshData(), 4);
	}
};

/**
 * TILE_FREE:			Tile was created, initialized, used, freed and is now waiting for usage again
 *						Runtime mesh sections do not exist
 *
 * TILE_INITIALIZED:	Tile was created and initialized (i.e. RMC were created)
 *						The runtime mesh component's mesh sections are not yet created
 *
 * TILE_FINISHED:		The runtime mesh component's mesh sections were created and are available
 *
 * TILE_UNDEFINED:		default value, state of every tile directly after creation until SetupTile is called
 *
 * TILE_TRANSITION:		Tile was freed and now the update position method was called
 *						Runtime mesh sections are not yet created
 */
UENUM(BlueprintType)
enum class ETileStatus : uint8
{
	TILE_FREE UMETA(DisplayName = "Free"),
	TILE_INITIALIZED UMETA(DisplayName = "Initialized"),
	TILE_FINISHED UMETA(DisplayName = "Finished"),
	TILE_UNDEFINED UMETA(DisplayName = "Undefined"),
	TILE_TRANSITION UMETA(DisplayName = "Transition")
};

/**
 * 
 */
UCLASS()
class HOVERTEST_API UMyStaticLibrary : public UObject
{
	GENERATED_BODY()
	
public:

	static void PrintActorArray(TArray<AActor*> ActorArray)
	{
		int32 i = 0;
		for (AActor* Actor : ActorArray)
		{
			UE_LOG(LogTemp, Warning, TEXT("%i: %s"), i, *Actor->GetName());
			i++;
		}
	}

	static void PrintFIntVector2DArray(TArray<FIntVector2D> VectorArray)
	{
		int32 i = 0;
		for (FIntVector2D Vector : VectorArray)
		{
			UE_LOG(LogTemp, Warning, TEXT("%i: %s"), i, *Vector.ToString());
			i++;
		}
	}

	static void CreateSimpleMeshData(FTerrainSettings TerrainSettings, TArray<FMeshData>& MeshDataOUT)
	{
		// First vertex
		FMeshData MeshData;
		MeshData.VertexBuffer.Add(FRuntimeMeshVertexSimple(FVector(0, 0, 0), FVector(0, 0, 1), FRuntimeMeshTangent(0, -1, 0), FColor::White, FVector2D(0, 0)));

		// second vertex
		MeshData.VertexBuffer.Add(FRuntimeMeshVertexSimple(FVector(TerrainSettings.TileSizeXUnits * TerrainSettings.UnitTileSize, 0, 0), FVector(0, 0, 1), FRuntimeMeshTangent(0, -1, 0), FColor::White, FVector2D(TerrainSettings.UnitTileSize, 0)));

		// third vertex
		MeshData.VertexBuffer.Add(FRuntimeMeshVertexSimple(FVector(TerrainSettings.TileSizeXUnits * TerrainSettings.UnitTileSize, TerrainSettings.TileSizeYUnits * TerrainSettings.UnitTileSize, 0), FVector(0, 0, 1), FRuntimeMeshTangent(0, -1, 0), FColor::White, FVector2D(TerrainSettings.UnitTileSize, TerrainSettings.UnitTileSize)));

		// fourth vertex
		MeshData.VertexBuffer.Add(FRuntimeMeshVertexSimple(FVector(0, TerrainSettings.TileSizeYUnits * TerrainSettings.UnitTileSize, 0), FVector(0, 0, 1), FRuntimeMeshTangent(0, -1, 0), FColor::White, FVector2D(0, TerrainSettings.UnitTileSize)));

		// Triangles
		MeshData.TriangleBuffer.Add(0);
		MeshData.TriangleBuffer.Add(2);
		MeshData.TriangleBuffer.Add(1);
		MeshData.TriangleBuffer.Add(3);
		MeshData.TriangleBuffer.Add(2);
		MeshData.TriangleBuffer.Add(0);
		MeshDataOUT.Add(MeshData);
	}

	static void CreateComplexMeshData(FTerrainSettings TerrainSettings, TArray<FMeshData>& MeshDataOUT)
	{
		FMeshData MeshData;
		for (int x = 0; x < TerrainSettings.TileSizeXUnits; ++x)
		{
			for (int y = 0; y < TerrainSettings.TileSizeYUnits; ++y)
			{
				float z = FMath::RandRange(-1.f, 1.f);
				FVector Vec = FVector(x * TerrainSettings.UnitTileSize, y * TerrainSettings.UnitTileSize, z * 10.f);
				MeshData.VertexBuffer.Add(FRuntimeMeshVertexSimple(Vec, FVector(0.f, 0.f, 1.f), FRuntimeMeshTangent(0, -1, 0), FColor::White, FVector2D(x, y)));
			}
		}

		int32 VertexIterator = 0;

		while (VertexIterator < ((TerrainSettings.TileSizeXUnits - 1) * TerrainSettings.TileSizeXUnits))
		{
			/* 2 vertices from "left", 1 vertex from "right" */
			MeshData.TriangleBuffer.Add(VertexIterator);
			MeshData.TriangleBuffer.Add(VertexIterator + 1);
			MeshData.TriangleBuffer.Add(VertexIterator + TerrainSettings.TileSizeXUnits);

			/* 2 vertices from "right", 1 vertex from "left" */
			MeshData.TriangleBuffer.Add(VertexIterator + 1);
			MeshData.TriangleBuffer.Add((VertexIterator + TerrainSettings.TileSizeXUnits) + 1);
			MeshData.TriangleBuffer.Add(VertexIterator + TerrainSettings.TileSizeXUnits);

			/* check if we reached the end of the column */
			if ((VertexIterator + 2) % TerrainSettings.TileSizeXUnits == 0)	// +2 because VertexIterator is 0-based
			{
				VertexIterator += 2;
			}
			else
			{
				VertexIterator += 1;
			}
		}

		MeshDataOUT.Add(MeshData);
	}

	//Float as String With Precision! || Code Taken from https://wiki.unrealengine.com/Float_as_String_With_Precision Author: Rama
	static FORCEINLINE FString GetFloatAsStringWithPrecision(float TheFloat, int32 Precision, bool IncludeLeadingZero = true)
	{
		////Round to integral if have something like 1.9999 within precision
		//float Rounded = roundf(TheFloat);
		//if (FMath::Abs(TheFloat - Rounded) < FMath::Pow(10, -1 * Precision))
		//{
		//	TheFloat = Rounded;
		//}
		//FNumberFormattingOptions NumberFormat;					//Text.h
		//NumberFormat.MinimumIntegralDigits = (IncludeLeadingZero) ? 1 : 0;
		//NumberFormat.MaximumIntegralDigits = 10000;
		//NumberFormat.MinimumFractionalDigits = Precision;
		//NumberFormat.MaximumFractionalDigits = Precision;
		//return FText::AsNumber(TheFloat, &NumberFormat).ToString();

		// own code follows
		float Buffer = TheFloat * FMath::Pow(10, Precision);
		Buffer = FMath::FloorToFloat(Buffer);
		return FString::SanitizeFloat((Buffer / FMath::Pow(10, Precision)), Precision);
	}

	static FORCEINLINE float GetFloatWithPrecision(const float TheFloat, const int32 Precision)
	{
		float Buffer = TheFloat * FMath::Pow(10, Precision);
		Buffer = FMath::FloorToFloat(Buffer);
		return (Buffer / FMath::Pow(10, Precision));
	}

	static void SaveBuffersToFile(const TArray<FRuntimeMeshVertexSimple>& VertexBuffer, const TArray<int32>& TriangleBuffer)
	{
		FString SaveDirectory = "D:/Users/Julien/Documents/Unreal Engine Dumps";
		FString VertexFileName = "VertexBuffer.txt";
		FString TriangleFileName = "TriangleBuffer.txt";
		FString VertexContent = "";
		FString TriangleContent = "";
		int32 TriangleIndex = 0;

		// Vertex Buffer
		VertexContent.Append("-------------- Begin of VertexBuffer --------------\n");
		for (int32 VertexIndex = 0; VertexIndex < VertexBuffer.Num(); ++VertexIndex)
		{
			VertexContent.Append(FString::FromInt(VertexIndex) + ": " + VertexBuffer[VertexIndex].Position.ToString() + "\n");
		}
		VertexContent.Append("-------------- End of VertexBuffer --------------\n");

		TriangleContent.Append("-------------- Begin of TriangleBuffer --------------\n");
		// Triangle Buffer
		for (const int32 TrianglePoint : TriangleBuffer)
		{
			if (TriangleIndex < 2)
			{
				TriangleContent.Append(FString::FromInt(TrianglePoint) + ", ");
				++TriangleIndex;
			}
			else
			{
				TriangleContent.Append(FString::FromInt(TrianglePoint) + "\n----------------\n");
				TriangleIndex = 0;
			}
		}
		TriangleContent.Append("-------------- End of TriangleBuffer --------------\n");

		bool AllowOverwriting = true;
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		// CreateDirectoryTree returns true if the destination
		// directory existed prior to call or has been created
		// during the call.
		if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
		{
			FString VertexAbsolutePath = SaveDirectory + "/" + VertexFileName;
			FString TriangleAbsolutePath = SaveDirectory + "/" + TriangleFileName;

			if (AllowOverwriting || !PlatformFile.FileExists(*VertexAbsolutePath))
			{
				FFileHelper::SaveStringToFile(VertexContent, *VertexAbsolutePath);
			}

			if (AllowOverwriting || !PlatformFile.FileExists(*TriangleAbsolutePath))
			{
				FFileHelper::SaveStringToFile(TriangleContent, *TriangleAbsolutePath);
			}
		}

		return;

	}

	/**
	 * calculates the minimum distance between a point and a given line segment
	 * code adopted from https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
	 * @param Point The Point for which the distance to the given line segment should be calculated
	 * @param LineStartPoint The start point of the line segment
	 * @param LineEndPoint The end point of the line segment
	 * @return Minimum distance between given point and line segment
	 */
	static float CalculateMinimumDistancePointLineSegment(const FVector2D Point, const FVector2D LineStartPoint, const FVector2D LineEndPoint)
	{
		const float L2 = FVector2D::DistSquared(LineStartPoint, LineEndPoint);
		if (FMath::IsNearlyZero(L2))
		{
			return FVector2D::Distance(Point, LineStartPoint);
		}
		const float t = FMath::Max<float>(0.f, FMath::Min<float>(1.f, FVector2D::DotProduct(Point - LineStartPoint, LineEndPoint - LineStartPoint) / L2));
		
		const FVector2D Projection = LineStartPoint + t * (LineEndPoint - LineStartPoint);
		return FVector2D::Distance(Point, Projection);
	}

	/**
	 * normal distribution whose value is optionally clamped between Min and Max
	 * @param Mean The mean
	 * @param Deviation The standard deviation
	 * @param Min If the value should be clamped to a minimum, set to 0 for no clamping
	 * @param Max If the value should be clamped to a maximum, set to 0 for no clamping
	 * @return A normal distributed value with mean Mean and standard deviation Deviation, optionally clamped to [Min, Max]
	 */
	static float GetNormalDistribution(const float Mean, const float Deviation, const float Min = 0.f, const float Max = 0.f)
	{
		// following the code from https://en.cppreference.com/w/cpp/numeric/random/normal_distribution
		std::random_device rd{};
		std::mt19937 gen{ rd() };

		std::normal_distribution<float> d{ Mean, Deviation };
		if (Min == 0.f && Max == 0.f)
		{
			return d(gen);
		}
		else
		{
			return FMath::Clamp<float>(d(gen), Min, Max);
		}
	}

	/**
	 * Calculates distance from the first point in the array to the last point in the array
	 * @param Traverse Array containing all points in the traverse in the right order
	 * @return The distance from the first point in the array to the last point
	 */
	static float CalculateTraverseDistance(const TArray<FVector>& Traverse)
	{
		float TraverseLength = 0.f;
		for (int32 i = 0; i < Traverse.Num() - 1; ++i)
		{
			TraverseLength += FVector::Distance(Traverse[i], Traverse[i + 1]);
		}

		return TraverseLength;
	}

	/**
	 * calculates the intersection point of two lines, each line is represented by two points that lie on it
	 * code taken from https://www.geeksforgeeks.org/program-for-point-of-intersection-of-two-lines/
	 * @param PointA First point that lies on line 1
	 * @param PointB Second point that lies on line 1
	 * @param PointC First point that lies on line 2
	 * @param PointC Second point that lies on line 2
	 * @param OUTIntersectionPoint The intersection point of the two lines
	 * @return If an intersection point could be calculated or not
	 */
	static bool CalculateIntersectionPoint(const FVector PointA, const FVector PointB, const FVector PointC, const FVector PointD, FVector2D& OUTIntersectionPoint)
	{
		// line AB represented as a1X + b1Y = c1
		double a1 = PointB.Y - PointA.Y;
		double b1 = PointA.X - PointB.X;
		double c1 = a1 * PointA.X + b1 * PointA.Y;

		// line CD represented as a2X + b2Y = c2
		double a2 = PointD.Y - PointC.Y;
		double b2 = PointC.X - PointD.X;
		double c2 = a2 * PointC.X + b2 * PointC.Y;

		double det = a1 * b2 - a2 * b1;

		if (FMath::IsNearlyZero(det))
		{
			return false;
		}
		else
		{
			OUTIntersectionPoint.X = (b2 * c1 - b1 * c2) / det;
			OUTIntersectionPoint.Y = (a1 * c2 - a2 * c1) / det;
			return true;
		}
	}

	/**
	* calculates the intersection point of two lines, each line is represented by two points that lie on it
	* code taken from https://www.geeksforgeeks.org/program-for-point-of-intersection-of-two-lines/
	* @param PointA First point that lies on line 1
	* @param PointB Second point that lies on line 1
	* @param PointC First point that lies on line 2
	* @param PointC Second point that lies on line 2
	* @param OUTIntersectionPoint The intersection point of the two lines
	* @return If an intersection point could be calculated or not
	*/
	static bool CalculateIntersectionPoint(const FVector2D PointA, const FVector2D PointB, const FVector2D PointC, const FVector2D PointD, FVector2D& OUTIntersectionPoint)
	{
		// line AB represented as a1X + b1Y = c1
		double a1 = PointB.Y - PointA.Y;
		double b1 = PointA.X - PointB.X;
		double c1 = a1 * PointA.X + b1 * PointA.Y;

		// line CD represented as a2X + b2Y = c2
		double a2 = PointD.Y - PointC.Y;
		double b2 = PointC.X - PointD.X;
		double c2 = a2 * PointC.X + b2 * PointC.Y;

		double det = a1 * b2 - a2 * b1;

		if (FMath::IsNearlyZero(det))
		{
			return false;
		}
		else
		{
			OUTIntersectionPoint.X = (b2 * c1 - b1 * c2) / det;
			OUTIntersectionPoint.Y = (a1 * c2 - a2 * c1) / det;
			return true;
		}
	}

	static ETileBorder GuessTileBorder(const FVector PointOnBorder, float TileSize)
	{
		// bottom?
		if (PointOnBorder.X < (0.25 * TileSize) && PointOnBorder.Y >(0.25 * TileSize) && PointOnBorder.Y < (0.75 * TileSize))
		{
			return ETileBorder::ETB_Bottom;
		}
		// right?
		else if (PointOnBorder.X < (0.75 * TileSize) && PointOnBorder.X >(0.25 * TileSize) && PointOnBorder.Y >(0.75 * TileSize))
		{
			return ETileBorder::ETB_Right;
		}
		// top?
		else if (PointOnBorder.X > (0.75 * TileSize) && PointOnBorder.Y > (0.25 * TileSize) && PointOnBorder.Y < (0.75 * TileSize))
		{
			return ETileBorder::ETB_Top;
		}
		// left?
		else if (PointOnBorder.X >(0.25 * TileSize) && PointOnBorder.X < (0.75 * TileSize) && PointOnBorder.Y < (0.25 * TileSize))
		{
			return ETileBorder::ETB_Left;
		}
		else
		{
			return ETileBorder::ETB_Invalid;
		}
	}


};
	
