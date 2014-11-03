#include "hoNDArray_reductions.h"
#include "hoArmadillo.h"

namespace Gadgetron{

template<class REAL> REAL max(hoNDArray<REAL>* data){
	return as_arma_col(data).max();
}
template<class REAL> REAL min(hoNDArray<REAL>* data){
	return as_arma_col(data).min();
}


template<class T> T mean(hoNDArray<T>* data){
	return (typename stdType<T>::Type) arma::mean(as_arma_col(data));
}


template<class T> T sum(hoNDArray<T>* data){
	return (typename stdType<T>::Type) arma::sum(as_arma_col(data));
}

template<class T> T stddev(hoNDArray<T>* data){
	return (typename stdType<T>::Type) arma::stddev(as_arma_col(data));
}
template<class T> T dot( hoNDArray<T> *x, hoNDArray<T> *y, bool cc )
{
	if( x == 0x0 || y == 0x0 )
		throw std::runtime_error("Gadgetron::dot(): Invalid input array");

	if( x->get_number_of_elements() != y->get_number_of_elements() )
		throw std::runtime_error("Gadgetron::dot(): Array sizes mismatch");

	arma::Col<typename stdType<T>::Type> xM = as_arma_col(x);
	arma::Col<typename stdType<T>::Type> yM = as_arma_col(y);
	typename stdType<T>::Type res = (cc) ? arma::cdot(xM,yM) : arma::dot(xM,yM);
	return *((T*)(&res));
}

template<class T> typename realType<T>::Type asum( hoNDArray<T> *x )
{
	if( x == 0x0 )
		throw std::runtime_error("Gadgetron::asum(): Invalid input array");

	typedef typename realType<T>::Type realT;
	arma::Col<typename stdType<T>::Type> xM = as_arma_col(x);
	return realT(arma::norm(xM,1));
}

template<class T> T asum( hoNDArray< std::complex<T> > *x )
{
	if( x == 0x0 )
		throw std::runtime_error("Gadgetron::asum(): Invalid input array");

	return arma::norm(arma::abs(real(as_arma_col(x)))+arma::abs(imag(as_arma_col(x))),1);
}

template<class T> T asum( hoNDArray< complext<T> > *x )
{
	if( x == 0x0 )
		throw std::runtime_error("Gadgetron::asum(): Invalid input array");

	return arma::norm(arma::abs(real(as_arma_col(x)))+arma::abs(imag(as_arma_col(x))),1);
}

template<class T> typename realType<T>::Type nrm1( hoNDArray<T> *x )
{
	if( x == 0x0 )
		throw std::runtime_error("Gadgetron::nrm2(): Invalid input array");

	typedef typename realType<T>::Type realT;
	arma::Col<typename stdType<T>::Type> xM = as_arma_col(x);
	return realT(arma::norm(xM,1));
}

template<class T> typename realType<T>::Type nrm2( hoNDArray<T> *x )
{
	if( x == 0x0 )
		throw std::runtime_error("Gadgetron::nrm2(): Invalid input array");

	typedef typename realType<T>::Type realT;
	arma::Col<typename stdType<T>::Type> xM = as_arma_col(x);
	return realT(arma::norm(xM,2));
}

template<class T> size_t amin( hoNDArray<T> *x )
{
	if( x == 0x0 )
		throw std::runtime_error("Gadgetron::amin(): Invalid input array");

	typedef typename realType<T>::Type realT;
	arma::Col<realT> xM = arma::abs(as_arma_col(x));
	arma::uword idx;
	realT min = xM.min(idx);
	return idx;
}

template<class T> size_t amin( hoNDArray< std::complex<T> > *x )
{
	if( x == 0x0 )
		throw std::runtime_error("Gadgetron::amin(): Invalid input array");

	arma::Col<T> xM = arma::abs(real(as_arma_col(x)))+arma::abs(imag(as_arma_col(x)));
	arma::uword idx;
	T min = xM.min(idx);
	return idx;
}

template<class T> size_t amin( hoNDArray< complext<T> > *x )
{
	if( x == 0x0 )
		throw std::runtime_error("Gadgetron::amin(): Invalid input array");

	arma::Col<T> xM = arma::abs(real(as_arma_col(x)))+arma::abs(imag(as_arma_col(x)));
	arma::uword idx;
	T min = xM.min(idx);
	return idx;
}

template<class T> size_t amax( hoNDArray<T> *x )
{
	if( x == 0x0 )
		throw std::runtime_error("Gadgetron::amax(): Invalid input array");

	typedef typename realType<T>::Type realT;
	arma::Col<realT> xM = arma::abs(as_arma_col(x));
	arma::uword idx;
	realT max = xM.max(idx);
	return idx;
}

template<class T> size_t amax( hoNDArray< std::complex<T> > *x )
{
	if( x == 0x0 )
		throw std::runtime_error("Gadgetron::amax(): Invalid input array");

	arma::Col<T> xM = arma::abs(real(as_arma_col(x)))+arma::abs(imag(as_arma_col(x)));
	arma::uword idx;
	T max = xM.max(idx);
	return idx;
}

template<class T> size_t amax( hoNDArray< complext<T> > *x )
{
	if( x == 0x0 )
		throw std::runtime_error("Gadgetron::amax(): Invalid input array");

	arma::Col<T> xM = arma::abs(real(as_arma_col(x)))+arma::abs(imag(as_arma_col(x)));
	arma::uword idx;
	T max = xM.max(idx);
	return idx;
}
template EXPORTCPUCOREMATH float max(hoNDArray<float>*);
template EXPORTCPUCOREMATH float min(hoNDArray<float>*);
template EXPORTCPUCOREMATH float mean(hoNDArray<float>*);
template EXPORTCPUCOREMATH float sum(hoNDArray<float>*);
template EXPORTCPUCOREMATH float stddev(hoNDArray<float>*);

template EXPORTCPUCOREMATH double max(hoNDArray<double>*);
template EXPORTCPUCOREMATH double min(hoNDArray<double>*);
template EXPORTCPUCOREMATH double mean(hoNDArray<double>*);
template EXPORTCPUCOREMATH double sum(hoNDArray<double>*);
template EXPORTCPUCOREMATH double stddev(hoNDArray<double>*);

template EXPORTCPUCOREMATH complext<double> mean(hoNDArray<complext<double> >*);
template EXPORTCPUCOREMATH complext<double> sum(hoNDArray<complext<double> >*);

template EXPORTCPUCOREMATH complext<float> mean(hoNDArray<complext<float> >*);
template EXPORTCPUCOREMATH complext<float> sum(hoNDArray<complext<float> >*);


template EXPORTCPUCOREMATH float dot<float>( hoNDArray<float>*, hoNDArray<float>*, bool );
template EXPORTCPUCOREMATH float asum<float>( hoNDArray<float>* );
template EXPORTCPUCOREMATH float nrm2<float>( hoNDArray<float>* );

template EXPORTCPUCOREMATH size_t amin<float>( hoNDArray<float>* );
template EXPORTCPUCOREMATH size_t amax<float>( hoNDArray<float>* );

template EXPORTCPUCOREMATH double dot<double>( hoNDArray<double>*, hoNDArray<double>*, bool );
template EXPORTCPUCOREMATH double asum<double>( hoNDArray<double>* );
template EXPORTCPUCOREMATH double nrm2<double>( hoNDArray<double>* );

template EXPORTCPUCOREMATH size_t amin<double>( hoNDArray<double>* );
template EXPORTCPUCOREMATH size_t amax<double>( hoNDArray<double>* );

template EXPORTCPUCOREMATH std::complex<float> dot< std::complex<float> >( hoNDArray< std::complex<float> >*, hoNDArray< std::complex<float> >*, bool );
template EXPORTCPUCOREMATH float asum<float>( hoNDArray< std::complex<float> >* );
template EXPORTCPUCOREMATH float nrm2< std::complex<float> >( hoNDArray< std::complex<float> >* );
template EXPORTCPUCOREMATH float nrm1< std::complex<float> >( hoNDArray< std::complex<float> >* );
template EXPORTCPUCOREMATH size_t amin<float>( hoNDArray< std::complex<float> >* );
template EXPORTCPUCOREMATH size_t amax<float>( hoNDArray< std::complex<float> >* );

template EXPORTCPUCOREMATH std::complex<double> dot< std::complex<double> >( hoNDArray< std::complex<double> >*, hoNDArray< std::complex<double> >*, bool );
template EXPORTCPUCOREMATH double asum<double>( hoNDArray< std::complex<double> >* );
template EXPORTCPUCOREMATH double nrm2< std::complex<double> >( hoNDArray< std::complex<double> >* );
template EXPORTCPUCOREMATH double nrm1< std::complex<double> >( hoNDArray< std::complex<double> >* );
template EXPORTCPUCOREMATH size_t amin<double>( hoNDArray< std::complex<double> >* );
template EXPORTCPUCOREMATH size_t amax<double>( hoNDArray< std::complex<double> >* );

template EXPORTCPUCOREMATH complext<float> dot< complext<float> >( hoNDArray< complext<float> >*, hoNDArray< complext<float> >*, bool );
template EXPORTCPUCOREMATH float asum<float>( hoNDArray< complext<float> >* );
template EXPORTCPUCOREMATH float nrm2< complext<float> >( hoNDArray< complext<float> >* );
template EXPORTCPUCOREMATH float nrm1< complext<float> >( hoNDArray< complext<float> >* );
template EXPORTCPUCOREMATH size_t amin<float>( hoNDArray< complext<float> >* );
template EXPORTCPUCOREMATH size_t amax<float>( hoNDArray< complext<float> >* );

template EXPORTCPUCOREMATH complext<double> dot< complext<double> >( hoNDArray< complext<double> >*, hoNDArray< complext<double> >*, bool );
template EXPORTCPUCOREMATH double asum<double>( hoNDArray< complext<double> >* );
template EXPORTCPUCOREMATH double nrm2< complext<double> >( hoNDArray< complext<double> >* );
template EXPORTCPUCOREMATH size_t amin<double>( hoNDArray< complext<double> >* );
template EXPORTCPUCOREMATH size_t amax<double>( hoNDArray< complext<double> >* );

}
