#include "Ocean.h"

namespace dyno
{
	template<typename TDataType>
	Ocean<TDataType>::Ocean()
		: Node()
	{
		auto heights = std::make_shared<HeightField<TDataType>>();
		this->stateHeightField()->setDataPtr(heights);
	}

	template<typename TDataType>
	Ocean<TDataType>::~Ocean()
	{
		this->varExtentX()->setRange(1, 10);
		this->varExtentZ()->setRange(1, 10);
	}

	template<typename TDataType>
	void Ocean<TDataType>::resetStates()
	{
		auto patch = this->getOceanPatch();

		auto Nx = this->varExtentX()->getData();
		auto Nz = this->varExtentZ()->getData();

		auto patchHeights = patch->stateHeightField()->getDataPtr();
		auto oceanHeights = this->stateHeightField()->getDataPtr();

		Real h = patchHeights->getGridSpacing();
		oceanHeights->setExtents(Nx * patchHeights->width(), Nz * patchHeights->height());
		oceanHeights->setGridSpacing(h);
		oceanHeights->setOrigin(Vec3f(-0.5 * h * oceanHeights->width(), 0, -0.5 * h * oceanHeights->height()));

		Real level = this->varWaterLevel()->getValue();

		//Initialize the height field for the ocean
		DArray2D<Coord>& patchDisp = patchHeights->getDisplacement();
		cuExecute2D(make_uint2(patchDisp.nx(), patchDisp.ny()),
			O_InitOceanWave,
			oceanHeights->getDisplacement(),
			patchDisp,
			level);
	}

	template<typename Real, typename Coord>
	__global__ void O_InitOceanWave(
		DArray2D<Coord> oceanVertex,
		DArray2D<Coord> displacement,
		Real level)	//Water level
	{
		int i = threadIdx.x + blockIdx.x * blockDim.x;
		int j = threadIdx.y + blockIdx.y * blockDim.y;

		int width = displacement.nx();
		int height = displacement.ny();

		if (i < width && j < height)
		{
			Coord D_ij = displacement(i, j);

			int tiledX = oceanVertex.nx() / displacement.nx();
			int tiledY = oceanVertex.ny() / displacement.ny();
			for (int t = 0; t < tiledX; t++)
			{
				for (int s = 0; s < tiledY; s++)
				{
					int nx = i + t * width;
					int ny = j + s * height;

					oceanVertex(nx, ny) = D_ij;
					oceanVertex(nx, ny).y += level;
				}
			}
		}
	}

	template<typename Coord>
	__global__ void O_AddOceanTrails(
		DArray2D<Coord> oceanVertex,
		DArray2D<Coord> CapillaryWave)
	{
		int i = threadIdx.x + blockIdx.x * blockDim.x;
		int j = threadIdx.y + blockIdx.y * blockDim.y;

		int width = CapillaryWave.nx();
		int height = CapillaryWave.ny();

		if (i < width && j < height)
		{
			Coord C_ij = CapillaryWave(i, j);

			int tiledX = oceanVertex.nx() / CapillaryWave.nx();
			int tiledY = oceanVertex.ny() / CapillaryWave.ny();

			//TODO: correct the position
			int nx = i;
			int ny = j;

			oceanVertex(nx, ny) += C_ij;
		}
	}

	template<typename TDataType>
	void Ocean<TDataType>::updateStates()
	{
		Real level = this->varWaterLevel()->getValue();

		auto patch = this->getOceanPatch();

		auto patchHeights = patch->stateHeightField()->getDataPtr();

		auto oceanHeights = this->stateHeightField()->getDataPtr();

		//Initialize the height field for the ocean
		DArray2D<Coord>& patchDisp = patchHeights->getDisplacement();
		cuExecute2D(make_uint2(patchDisp.nx(), patchDisp.ny()),
			O_InitOceanWave,
			oceanHeights->getDisplacement(),
			patchDisp,
			level);

		//Add capillary waves
		auto& waves = this->getCapillaryWaves();
		for (int i = 0; i < waves.size(); i++) {
			auto wave = this->stateHeightField()->getDataPtr();

			auto& waveDisp = wave->getDisplacement();

			cuExecute2D(make_uint2(waveDisp.nx(), waveDisp.ny()),
				O_AddOceanTrails,
				oceanHeights->getDisplacement(),
				waveDisp);
		}
	}

	template<typename TDataType>
	bool Ocean<TDataType>::validateInputs()
	{
		return this->getOceanPatch() != nullptr;
	}

	DEFINE_CLASS(Ocean);
}

