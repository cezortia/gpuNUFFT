
#include "texture_gridding_operator.hpp"

void GriddingND::TextureGriddingOperator::initKernel()
{
  IndType kernelSize = (interpolationType > 1) ? calculateKernelSizeLinInt(osf, kernelWidth/2.0f) : calculateGrid3KernelSize(osf, kernelWidth/2.0f);
  this->kernel.dim.width = kernelSize;
  this->kernel.dim.height = interpolationType > 1 ? kernelSize : 1;
  this->kernel.dim.depth = interpolationType > 2 ? kernelSize : 1;
  this->kernel.data = (DType*) calloc(this->kernel.count(),sizeof(DType));

  switch (interpolationType)
  {
    case TEXTURE_LOOKUP:   load1DKernel(this->kernel.data,(int)kernelSize,(int)kernelWidth,osf);break;
    case TEXTURE2D_LOOKUP:   load2DKernel(this->kernel.data,(int)kernelSize,(int)kernelWidth,osf);break;
    case TEXTURE3D_LOOKUP:   load3DKernel(this->kernel.data,(int)kernelSize,(int)kernelWidth,osf);break;
    default: load1DKernel(this->kernel.data,(int)kernelSize,(int)kernelWidth,osf);
  }

}

const char* GriddingND::TextureGriddingOperator::getInterpolationTypeName()
{
  switch (interpolationType)
  {
  case TEXTURE_LOOKUP:   return "texKERNEL";
  case TEXTURE2D_LOOKUP:   return "texKERNEL2D";
  case TEXTURE3D_LOOKUP:   return "texKERNEL3D";
  default: return "KERNEL";
  }

}

GriddingND::GriddingInfo* GriddingND::TextureGriddingOperator::initAndCopyGriddingInfo()
{
  GriddingND::GriddingInfo* gi_host = initGriddingInfo();

  gi_host->interpolationType = interpolationType;

  if (DEBUG)
    printf("copy Gridding Info to symbol memory... size = %ld \n",sizeof(GriddingND::GriddingInfo));

  initConstSymbol("GI",gi_host,sizeof(GriddingND::GriddingInfo));

  if (DEBUG)
    printf("...done!\n");
  return gi_host;
}

void GriddingND::TextureGriddingOperator::adjConvolution(DType2* data_d, 
      DType* crds_d, 
      CufftType* gdata_d,
      DType* kernel_d, 
      IndType* sectors_d, 
      IndType* sector_centers_d,
  GriddingND::GriddingInfo* gi_host)
{
  bindTo1DTexture("texDATA",data_d,this->kSpaceTraj.count());

  performTextureConvolution(data_d,crds_d,gdata_d,kernel_d,sectors_d,sector_centers_d,gi_host);

  unbindTexture("texDATA");
}

void GriddingND::TextureGriddingOperator::forwardConvolution(CufftType*		data_d, 
  DType*			crds_d, 
  CufftType*		gdata_d,
  DType*			kernel_d, 
  IndType*		sectors_d, 
  IndType*		sector_centers_d,
  GriddingND::GriddingInfo* gi_host)
{
  bindTo1DTexture("texGDATA",gdata_d,gi_host->grid_width_dim);

  performTextureForwardConvolution(data_d,crds_d,gdata_d,kernel_d,sectors_d,sector_centers_d,gi_host);

  unbindTexture("texGDATA");
}

void GriddingND::TextureGriddingOperator::initLookupTable()
{
  initTexture(getInterpolationTypeName(),&kernel_d,this->kernel);
}

void GriddingND::TextureGriddingOperator::freeLookupTable()
{
	freeTexture(getInterpolationTypeName(),kernel_d);
}