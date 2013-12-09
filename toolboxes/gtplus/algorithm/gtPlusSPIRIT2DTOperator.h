/** \file       gtPlusSPIRIT2DTOperator.h
    \brief      Base class for gtPlus 2DT operators
    \author     Hui Xue
*/

#pragma once

#include "gtPlusSPIRIT2DOperator.h"

namespace Gadgetron { namespace gtPlus {

template <typename T> 
class gtPlusSPIRIT2DTOperator : public gtPlusSPIRIT2DOperator<T>
{
public:

    typedef gtPlusSPIRIT2DOperator<T> BaseClass;

    gtPlusSPIRIT2DTOperator() : BaseClass() {}
    virtual ~gtPlusSPIRIT2DTOperator() {}

    virtual void printInfo(std::ostream& os);

    // set forward kernel, compute the adjoint and adjoint_forward kernel
    bool setForwardKernel(boost::shared_ptr< hoNDArray<T> >& forward_kernel, bool computeAdjForwardKernel=true);
    bool setAdjointForwardKernel(boost::shared_ptr< hoNDArray<T> >& adjoint_forward_kernel);
    // set the acquired kspace, unacquired points are set to be zero
    bool setAcquiredPoints(boost::shared_ptr< hoNDArray<T> >& kspace);

    // compute gradient of ||(G-I)(Dc'x+D'y)||2
    virtual bool grad(const hoNDArray<T>& x, hoNDArray<T>& g);

    // compute cost value of L2 norm ||(G-I)(Dc'x+D'y)||2
    virtual bool obj(const hoNDArray<T>& x, T& obj);

    //using BaseClass::gt_timer1_;
    //using BaseClass::gt_timer2_;
    //using BaseClass::gt_timer3_;
    //using BaseClass::performTiming_;
    //using BaseClass::gt_exporter_;
    //using BaseClass::debugFolder_;
    //using BaseClass::gtPlus_util_;
    //using BaseClass::gtPlus_util_complex_;
    //using BaseClass::gtPlus_mem_manager_;
    //using BaseClass::use_symmetric_spirit_;

protected:

    // G-I, [RO E1 srcCHA dstCHA N]
    //using BaseClass::forward_kernel_;
    //using BaseClass::adjoint_kernel_;
    //using BaseClass::adjoint_forward_kernel_;
    //using BaseClass::acquired_points_;
    //using BaseClass::acquired_points_indicator_;
    //using BaseClass::unacquired_points_indicator_;

    // helper memory
    //using BaseClass::kspace_;
    //using BaseClass::complexIm_;
    //using BaseClass::res_after_apply_kernel_;
    //using BaseClass::res_after_apply_kernel_sum_over_;

    //using BaseClass::kspace_Managed_;
    //using BaseClass::complexIm_Managed_;
    //using BaseClass::res_after_apply_kernel_Managed_;
    //using BaseClass::res_after_apply_kernel_sum_over_Managed_;
};

template <typename T> 
void gtPlusSPIRIT2DTOperator<T>::printInfo(std::ostream& os)
{
    using namespace std;

    os << "-------------- GTPlus ISMRMRD SPIRIT 2DT operator ------------------" << endl;
    os << "Implementation of SPIRIT 2DT operator for ISMRMRD package" << endl;
    os << "----------------------------------------------------------------------" << endl;
}

template <typename T> 
bool gtPlusSPIRIT2DTOperator<T>::
setForwardKernel(boost::shared_ptr< hoNDArray<T> >& forward_kernel, bool computeAdjForwardKernel)
{
    try
    {
        this->forward_kernel_ = forward_kernel;

        unsigned long long RO = this->forward_kernel_->get_size(0);
        unsigned long long E1 = this->forward_kernel_->get_size(1);
        unsigned long long srcCHA = this->forward_kernel_->get_size(2);
        unsigned long long dstCHA = this->forward_kernel_->get_size(3);
        unsigned long long N = this->forward_kernel_->get_size(4);

        this->adjoint_kernel_ = boost::shared_ptr< hoNDArray<T> >(new hoNDArray<T>(RO, E1, dstCHA, srcCHA, N));

        bool computeAdjointForwardKernel = (computeAdjForwardKernel || this->use_symmetric_spirit_);

        if ( computeAdjointForwardKernel )
        {
            this->adjoint_forward_kernel_ = boost::shared_ptr< hoNDArray<T> >(new hoNDArray<T>(RO, E1, dstCHA, dstCHA, N));
        }

        unsigned long long n;
        for ( n=0; n<N; n++ )
        {
            hoNDArray<T> kerCurr(RO, E1, srcCHA, dstCHA, this->forward_kernel_->begin()+n*RO*E1*srcCHA*dstCHA);
            hoNDArray<T> adjKerCurr(RO, E1, dstCHA, srcCHA, this->adjoint_kernel_->begin()+n*RO*E1*dstCHA*srcCHA);

            GADGET_CHECK_RETURN_FALSE(this->imageDomainAdjointKernel(kerCurr, adjKerCurr));

            if ( computeAdjointForwardKernel )
            {
                hoNDArray<T> adjForwardKerCurr(RO, E1, dstCHA, dstCHA, this->adjoint_forward_kernel_->begin()+n*RO*E1*dstCHA*dstCHA);
                GADGET_CHECK_RETURN_FALSE(this->AdjointForwardKernel(adjKerCurr, kerCurr, adjForwardKerCurr));
            }
        }
    }
    catch(...)
    {
        GADGET_ERROR_MSG("Errors in gtPlusSPIRIT2DTOperator<T>::setForwardKernel(...) ... ");
        return false;
    }

    return true;
}

template <typename T> 
bool gtPlusSPIRIT2DTOperator<T>::
setAdjointForwardKernel(boost::shared_ptr< hoNDArray<T> >& adjoint_forward_kernel)
{
    try
    {
        this->adjoint_forward_kernel_ = adjoint_forward_kernel;
    }
    catch(...)
    {
        GADGET_ERROR_MSG("Errors in gtPlusSPIRIT2DTOperator<T>::setAdjointForwardKernel(...) ... ");
        return false;
    }

    return true;
}

template <typename T> 
bool gtPlusSPIRIT2DTOperator<T>::
setAcquiredPoints(boost::shared_ptr< hoNDArray<T> >& kspace)
{
    try
    {
        this->acquired_points_ = kspace;

        unsigned long long RO = this->acquired_points_->get_size(0);
        unsigned long long E1 = this->acquired_points_->get_size(1);
        unsigned long long srcCHA = this->acquired_points_->get_size(2);
        unsigned long long E2 = this->acquired_points_->get_size(3);

        this->acquired_points_indicator_.create(kspace->get_dimensions());
        Gadgetron::clear(this->acquired_points_indicator_);

        this->unacquired_points_indicator_.create(kspace->get_dimensions());
        Gadgetron::clear(this->unacquired_points_indicator_);

        unsigned long long N = kspace->get_number_of_elements();

        long long ii;

        #ifdef GCC_OLD_FLAG
            #pragma omp parallel for default(none) private(ii) shared(N)
        #else
            #pragma omp parallel for default(none) private(ii) shared(N, kspace)
        #endif
        for ( ii=0; ii<(long long)N; ii++ )
        {
            if ( std::abs( (*kspace)(ii) ) < DBL_EPSILON )
            {
                this->unacquired_points_indicator_(ii) = 1.0;
            }
            else
            {
                this->acquired_points_indicator_(ii) = 1.0;
            }
        }

        // allocate the helper memory
        this->kspace_.create(RO, E1, srcCHA, E2);
        this->complexIm_.create(RO, E1, srcCHA, E2);

        if ( this->forward_kernel_ )
        {
            unsigned long long dstCHA = this->forward_kernel_->get_size(3);
            this->res_after_apply_kernel_.create(RO, E1, srcCHA, dstCHA);
            this->res_after_apply_kernel_sum_over_.create(RO, E1, dstCHA, E2);
        }
    }
    catch(...)
    {
        GADGET_ERROR_MSG("Errors in gtPlusSPIRIT2DTOperator<T>::setAcquiredPoints(...) ... ");
        return false;
    }

    return true;
}

template <typename T> 
bool gtPlusSPIRIT2DTOperator<T>::grad(const hoNDArray<T>& x, hoNDArray<T>& g)
{
    try
    {
        // gradient of L2 norm is
        // 2*Dc*(G-I)'(G-I)(D'y+Dc'x)

        // D'y+Dc'x
        GADGET_CHECK_RETURN_FALSE(Gadgetron::multiply(this->unacquired_points_indicator_, x, this->kspace_));
        GADGET_CHECK_RETURN_FALSE(Gadgetron::add(*this->acquired_points_, this->kspace_, this->kspace_));

        // x to image domain
        GADGET_CHECK_RETURN_FALSE(this->convertToImage(this->kspace_, this->complexIm_));

        // apply kernel and sum
        unsigned long long RO = x.get_size(0);
        unsigned long long E1 = x.get_size(1);
        unsigned long long CHA = x.get_size(2);
        unsigned long long N = x.get_size(3);

        unsigned long long dstCHA = this->adjoint_forward_kernel_->get_size(3);
        unsigned long long kernelN = this->adjoint_forward_kernel_->get_size(4);

        this->res_after_apply_kernel_sum_over_.create(RO, E1, dstCHA, N);

        unsigned long long n;
        for ( n=0; n<N; n++)
        {
            hoNDArray<T> currComplexIm(RO, E1, CHA, this->complexIm_.begin()+n*RO*E1*CHA);

            hoNDArray<T> curr_adjoint_forward_kernel;

            if ( n < kernelN )
            {
                curr_adjoint_forward_kernel.create(RO, E1, CHA, dstCHA, this->adjoint_forward_kernel_->begin()+n*RO*E1*CHA*dstCHA);
            }
            else
            {
                curr_adjoint_forward_kernel.create(RO, E1, CHA, dstCHA, this->adjoint_forward_kernel_->begin()+(kernelN-1)*RO*E1*CHA*dstCHA);
            }

            GADGET_CHECK_RETURN_FALSE(Gadgetron::multipleMultiply(currComplexIm, curr_adjoint_forward_kernel, this->res_after_apply_kernel_));

            hoNDArray<T> sumResCurr(RO, E1, dstCHA, this->res_after_apply_kernel_sum_over_.begin()+n*RO*E1*dstCHA);
            GADGET_CHECK_RETURN_FALSE(Gadgetron::sumOverSecondLastDimension(this->res_after_apply_kernel_, sumResCurr));
        }

        // go back to kspace 
        GADGET_CHECK_RETURN_FALSE(this->convertToKSpace(this->res_after_apply_kernel_sum_over_, g));

        // apply Dc
        GADGET_CHECK_RETURN_FALSE(Gadgetron::multiply(this->unacquired_points_indicator_, g, g));

        // multiply by 2
        GADGET_CHECK_RETURN_FALSE(Gadgetron::scal(T(2.0), g));
    }
    catch(...)
    {
        GADGET_ERROR_MSG("Errors in gtPlusSPIRIT2DTOperator<T>::grad(...) ... ");
        return false;
    }

    return true;
}

template <typename T> 
bool gtPlusSPIRIT2DTOperator<T>::obj(const hoNDArray<T>& x, T& obj)
{
    try
    {
        // L2 norm
        // ||(G-I)(D'y+Dc'x)||2

        // D'y+Dc'x
        GADGET_CHECK_RETURN_FALSE(Gadgetron::multiply(this->unacquired_points_indicator_, x, this->kspace_));
        GADGET_CHECK_RETURN_FALSE(Gadgetron::add(*this->acquired_points_, this->kspace_, this->kspace_));

        // x to image domain
        GADGET_CHECK_RETURN_FALSE(this->convertToImage(this->kspace_, this->complexIm_));

        // apply kernel and sum
        unsigned long long RO = x.get_size(0);
        unsigned long long E1 = x.get_size(1);
        unsigned long long CHA = x.get_size(2);
        unsigned long long N = x.get_size(3);

        unsigned long long dstCHA = this->forward_kernel_->get_size(3);
        unsigned long long kernelN = this->forward_kernel_->get_size(4);

        this->res_after_apply_kernel_sum_over_.create(RO, E1, dstCHA, N);

        unsigned long long n;
        for ( n=0; n<N; n++)
        {
            hoNDArray<T> currComplexIm(RO, E1, CHA, this->complexIm_.begin()+n*RO*E1*CHA);

            hoNDArray<T> curr_forward_kernel;

            if ( n < kernelN )
            {
                curr_forward_kernel.create(RO, E1, CHA, dstCHA, this->forward_kernel_->begin()+n*RO*E1*CHA*dstCHA);
            }
            else
            {
                curr_forward_kernel.create(RO, E1, CHA, dstCHA, this->forward_kernel_->begin()+(kernelN-1)*RO*E1*CHA*dstCHA);
            }

            GADGET_CHECK_RETURN_FALSE(Gadgetron::multipleMultiply(currComplexIm, curr_forward_kernel, this->res_after_apply_kernel_));

            hoNDArray<T> sumResCurr(RO, E1, dstCHA, this->res_after_apply_kernel_sum_over_.begin()+n*RO*E1*dstCHA);
            GADGET_CHECK_RETURN_FALSE(Gadgetron::sumOverSecondLastDimension(this->res_after_apply_kernel_, sumResCurr));
        }

        // L2 norm
        GADGET_CHECK_RETURN_FALSE(Gadgetron::dotc(this->res_after_apply_kernel_sum_over_, this->res_after_apply_kernel_sum_over_, obj));
    }
    catch(...)
    {
        GADGET_ERROR_MSG("Errors in gtPlusSPIRIT2DTOperator<T>::grad(...) ... ");
        return false;
    }

    return true;
}

}}
