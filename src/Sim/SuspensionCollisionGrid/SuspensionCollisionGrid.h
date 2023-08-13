#pragma once
#include "../Car/Car.h"

class btBvhTriangleMeshShape;

struct SuspensionCollisionGrid {
	constexpr static float
		EXTENT_X = RLConst::ARENA_EXTENT_X,
		EXTENT_Y = RLConst::ARENA_EXTENT_Y * 1.5f, // Extends into goal
		HEIGHT = RLConst::ARENA_HEIGHT;

	bool lightMem;

	constexpr static int
		CELL_AMOUNT_X[2] = { 128, 32 },
		CELL_AMOUNT_Y[2] = { 224, 56 },
		CELL_AMOUNT_Z[2] = { 32, 16 },
		CELL_AMOUNT_TOTAL[2] = { (CELL_AMOUNT_X[0] * CELL_AMOUNT_Y[0] * CELL_AMOUNT_Z[0]), (CELL_AMOUNT_X[1] * CELL_AMOUNT_Y[1] * CELL_AMOUNT_Z[1]) };

	constexpr static float
		CELL_SIZE_X[2] = { EXTENT_X / (CELL_AMOUNT_X[0] / 2), EXTENT_X / (CELL_AMOUNT_X[1] / 2) },
		CELL_SIZE_Y[2] = { EXTENT_Y / (CELL_AMOUNT_Y[0] / 2), EXTENT_Y / (CELL_AMOUNT_Y[1] / 2) },
		CELL_SIZE_Z[2] = { HEIGHT / CELL_AMOUNT_Z[0] , HEIGHT / CELL_AMOUNT_Z[1] };

	// Make sure cell sizes arent't too small, a ray shouldn't be able to travel through multiple cells
	static_assert(RS_MIN(CELL_SIZE_X[0], RS_MIN(CELL_SIZE_Y[0], CELL_SIZE_Z[0])) > 60, "Building collision suspension grid from arena meshes...");

	struct Cell {
		bool worldCollision = false;
		int dynamicObjects = 0;
	};

	SuspensionCollisionGrid(bool lightMem = false) : lightMem(lightMem) {}

	std::vector<Cell> cellData;

	void Allocate() {
		cellData.resize(CELL_AMOUNT_TOTAL[lightMem]);
	}

	template <bool LIGHT>
	Cell& Get(int i, int j, int k) {
		int index = (i * CELL_AMOUNT_Y[LIGHT] * CELL_AMOUNT_Z[LIGHT]) + (j * CELL_AMOUNT_Z[LIGHT]) + k;
		return cellData[index];
	}

	template <bool LIGHT>
	Cell Get(int i, int j, int k) const {
		int index = (i * CELL_AMOUNT_Y[LIGHT] * CELL_AMOUNT_Z[LIGHT]) + (j * CELL_AMOUNT_Z[LIGHT]) + k;
		return cellData[index];
	}

	template <bool LIGHT>
	Vec GetCellMin(int xIndex, int yIndex, int zIndex) const {
		return Vec(
			CELL_SIZE_X[LIGHT] * (xIndex - (CELL_AMOUNT_X[LIGHT] / 2)),
			CELL_SIZE_Y[LIGHT] * (yIndex - (CELL_AMOUNT_Y[LIGHT] / 2)),
			CELL_SIZE_Z[LIGHT] * zIndex
		);
	}

	template <bool LIGHT>
	void GetCellIndicesFromPos(Vec pos, int& i, int& j, int& k) const {
		i = RS_CLAMP(pos.x / CELL_SIZE_X[LIGHT] + (CELL_AMOUNT_X[LIGHT] / 2), 0, CELL_AMOUNT_X[LIGHT] - 1),
		j = RS_CLAMP(pos.y / CELL_SIZE_Y[LIGHT] + (CELL_AMOUNT_Y[LIGHT] / 2), 0, CELL_AMOUNT_Y[LIGHT] - 1),
		k = RS_CLAMP(pos.z / CELL_SIZE_Z[LIGHT], 0, CELL_AMOUNT_Z[LIGHT] - 1);
	}

	template <bool LIGHT>
	Cell& GetCellFromPos(Vec pos) {
		int i, j, k;
		GetCellIndicesFromPos<LIGHT>(pos, i, j, k);
		return Get<LIGHT>(i, j, k);
	}

	template <bool LIGHT>
	Vec GetCellSize() const {
		return Vec(CELL_SIZE_X[LIGHT], CELL_SIZE_Y[LIGHT], CELL_SIZE_Z[LIGHT]);
	}

	void SetupWorldCollision(const std::vector<btBvhTriangleMeshShape*>& triMeshShapes);

	btCollisionObject* CastSuspensionRay(btVehicleRaycaster* raycaster, Vec start, Vec end, btVehicleRaycaster::btVehicleRaycasterResult& result);
	void UpdateDynamicCollisions(Vec minBT, Vec maxBT, bool remove);

	btRigidBody* defaultWorldCollisionRB = NULL;
};
