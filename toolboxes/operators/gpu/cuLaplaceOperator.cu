#include "cuLaplaceOperator.h"
#include "cuNDArray_operators.h"
#include "cuNDArray_elemwise.h"
#include "vector_td.h"
#include "vector_td_utilities.h"
#include "check_CUDA.h"

namespace Gadgetron{

  // Template Power function
  template<unsigned int i, unsigned int j>
  struct Pow
  {
    enum { Value = i*Pow<i,j-1>::Value};
  };

  template <unsigned int i>
  struct Pow<i,1>
  {
    enum { Value = i};
  };

  template<class T, unsigned long long D, unsigned int dim> class inner_laplace_functor{
  public:
		static __device__ __inline__ void apply(T& val,const T* in, const typename intd<D>::Type dims,const typename intd<D>::Type co, typename intd<D>::Type& stride){
			for (int d = -1; d < 2; d++)
				stride[dim]=d;
				inner_laplace_functor<T,D,dim-1>::apply(val,in,dims,co,stride);
		}
  };
  template<class T, unsigned long long D> class inner_laplace_functor<T,D,0>{
  public:
  	static __device__ __inline__ void apply(T& val,const T* in, const typename intd<D>::Type dims,const typename intd<D>::Type co, typename intd<D>::Type& stride){
  		typename intd<D>::Type coN = (co+dims+stride)%dims;
  		val -= in[co_to_idx<D>(coN,dims)];
  	}
  };

  template<class REAL, class T, unsigned long long D> __global__ void
  laplace_kernel( typename intd<D>::Type dims, T *in, T *out )
  {  
    const int idx = blockIdx.y*gridDim.x*blockDim.x + blockIdx.x*blockDim.x + threadIdx.x;
    if( idx < prod(dims) ){
    
      T val = T(0);
      typename intd<D>::Type coN;

      typename intd<D>::Type co = idx_to_co<D>(idx, dims);

      typename intd<D>::Type stride(0);


      inner_laplace_functor<T,D,D-1>::apply(val,in,dims,co,stride);
      out[idx] = val+in[co_to_idx<D>(co, dims)]*((REAL) Pow<3,D>::Value);
    }
  }

  template< class T, unsigned long long D> void
  cuLaplaceOperator<T,D>::compute_laplace( cuNDArray<T> *in, cuNDArray<T> *out, bool accumulate )
  {
  
    if( !in || !out || in->get_number_of_elements() != out->get_number_of_elements() ){
      throw std::runtime_error("laplaceOperator::compute_laplace : array dimensions mismatch.");

    }
  
    typename intd<D>::Type dims = to_intd( from_std_vector<unsigned long long,D>( *(in->get_dimensions().get()) ));

    dim3 dimBlock( dims[0] );
    dim3 dimGrid( prod(dims)/dims[0] );
  
    // Invoke kernel
    laplace_kernel<typename realType<T>::Type ,T,D><<< dimGrid, dimBlock >>> (dims, in->get_data_ptr(), out->get_data_ptr() );
  
    CHECK_FOR_CUDA_ERROR();
  }
  
  // Instantiations

  template class EXPORTGPUOPERATORS cuLaplaceOperator<float, 1>;
  template class EXPORTGPUOPERATORS cuLaplaceOperator<float, 2>;
  template class EXPORTGPUOPERATORS cuLaplaceOperator<float, 3>;

  template class EXPORTGPUOPERATORS cuLaplaceOperator<float_complext, 1>;
  template class EXPORTGPUOPERATORS cuLaplaceOperator<float_complext, 2>;
  template class EXPORTGPUOPERATORS cuLaplaceOperator<float_complext, 3>;

  template class EXPORTGPUOPERATORS cuLaplaceOperator<double, 1>;
  template class EXPORTGPUOPERATORS cuLaplaceOperator<double, 2>;
  template class EXPORTGPUOPERATORS cuLaplaceOperator<double, 3>;

  template class EXPORTGPUOPERATORS cuLaplaceOperator<double_complext, 1>;
  template class EXPORTGPUOPERATORS cuLaplaceOperator<double_complext, 2>;
  template class EXPORTGPUOPERATORS cuLaplaceOperator<double_complext, 3>;
}
