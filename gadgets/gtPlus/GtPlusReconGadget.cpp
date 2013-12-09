
#include "GtPlusReconGadget.h"
#include "GtPlusGadgetOpenMP.h"

using namespace Gadgetron::gtPlus;

namespace Gadgetron
{

GtPlusReconGadget::GtPlusReconGadget() : mem_manager_(new Gadgetron::gtPlus::gtPlusMemoryManager(4, 640*1024*1024))
{
    image_series_ = 100;

    min_intensity_value_ = 64;
    max_intensity_value_ = 4095;

    max_intensity_value_US_ = 2048;

    scalingFactor_ = -1;
    use_constant_scalingFactor_ = false;

    timeStampResolution_ = 0.0025f;

    aSpacing_[0] = 2.0;
    aSpacing_[1] = 2.0;
    aSpacing_[2] = 6.0;
    aSpacing_[3] = 1.0;
    aSpacing_[4] = 1.0;
    aSpacing_[5] = 1.0;

    reconE1_ = 1;
    reconE2_ = 1;

    processed_called_times_ = 0;

    kSpaceMaxAcqE2No_ = 0;

    filterRO_type_ = ISMRMRD_FILTER_GAUSSIAN;
    filterRO_sigma_ = 1.5;
    filterRO_width_ = 0.15;

    filterE1_type_ = ISMRMRD_FILTER_GAUSSIAN;
    filterE1_sigma_ = 1.5;
    filterE1_width_ = 0.15;

    filterE2_type_ = ISMRMRD_FILTER_GAUSSIAN;
    filterE2_sigma_ = 1.5;
    filterE2_width_ = 0.15;

    filterRO_ref_type_ = ISMRMRD_FILTER_HANNING;
    filterRO_ref_sigma_ = 1.5;
    filterRO_ref_width_ = 0.15;

    filterE1_ref_type_ = ISMRMRD_FILTER_HANNING;
    filterE1_ref_sigma_ = 1.5;
    filterE1_ref_width_ = 0.15;

    filterE2_ref_type_ = ISMRMRD_FILTER_HANNING;
    filterE2_ref_sigma_ = 1.5;
    filterE2_ref_width_ = 0.15;

    filterRO_pf_type_ = ISMRMRD_FILTER_HANNING;
    filterRO_pf_sigma_ = 1.5;
    filterRO_pf_width_ = 0.15;
    filterRO_pf_densityComp_ = false;

    filterE1_pf_type_ = ISMRMRD_FILTER_HANNING;
    filterE1_pf_sigma_ = 1.5;
    filterE1_pf_width_ = 0.15;
    filterE1_pf_densityComp_ = false;

    filterE2_pf_type_ = ISMRMRD_FILTER_HANNING;
    filterE2_pf_sigma_ = 1.5;
    filterE2_pf_width_ = 0.15;
    filterE2_pf_densityComp_ = false;

    debugFolder_ = "DebugOutput";
    debugFolder2_ = debugFolder_;

    performTiming_ = true;

    verboseMode_ = false;

    CloudComputing_ = false;
    CloudSize_ = 0;

    gt_timer1_.set_timing_in_destruction(false);
    gt_timer2_.set_timing_in_destruction(false);
    gt_timer3_.set_timing_in_destruction(false);

    Gadgetron::prepOpenMP();
    Gadgetron::prepMKL();
}

GtPlusReconGadget::~GtPlusReconGadget()
{

}

bool GtPlusReconGadget::readParameters()
{
    try
    {
        GADGET_CONDITION_MSG(verboseMode_, "------> GtPlusReconGadget parameters <------");

        min_intensity_value_ = this->get_int_value("min_intensity_value");
        GADGET_CONDITION_MSG(verboseMode_, "min_intensity_value_ is " << min_intensity_value_);

        max_intensity_value_ = this->get_int_value("max_intensity_value");
        GADGET_CONDITION_MSG(verboseMode_, "max_intensity_value_ is " << max_intensity_value_);

        scalingFactor_ = this->get_double_value("scalingFactor");
        GADGET_CONDITION_MSG(verboseMode_, "scalingFactor_ is " << scalingFactor_);

        use_constant_scalingFactor_ = this->get_bool_value("use_constant_scalingFactor");
        GADGET_CONDITION_MSG(verboseMode_, "use_constant_scalingFactor_ is " << use_constant_scalingFactor_);

        boost::shared_ptr<std::string> str = this->get_string_value("debugFolder");
        debugFolder_ = *str;
        GADGET_CONDITION_MSG(verboseMode_, "debugFolder_ is " << debugFolder_);

        boost::shared_ptr<std::string> str2 = this->get_string_value("debugFolder2");
        debugFolder2_ = *str2;
        GADGET_CONDITION_MSG(verboseMode_, "debugFolder2_ is " << debugFolder2_);

        timeStampResolution_ = (float)this->get_double_value("timeStampResolution");
        GADGET_CONDITION_MSG(verboseMode_, "timeStampResolution_ is " << timeStampResolution_);

        performTiming_ = this->get_bool_value("performTiming");
        GADGET_CONDITION_MSG(verboseMode_, "performTiming_ is " << performTiming_);

        // kspace filter parameters
        str = this->get_string_value("filterRO");
        filterRO_type_ = gtPlus_util_.getISMRMRDKSpaceFilterFromName(*str);
        filterRO_sigma_ = this->get_double_value("filterRO_sigma");
        filterRO_width_ = this->get_double_value("filterRO_width");
        GADGET_CONDITION_MSG(verboseMode_, "filterRO_type_ is " << *str);
        GADGET_CONDITION_MSG(verboseMode_, "filterRO_sigma_ is " << filterRO_sigma_);
        GADGET_CONDITION_MSG(verboseMode_, "filterRO_width_ is " << filterRO_width_);

        str = this->get_string_value("filterE1");
        filterE1_type_ = gtPlus_util_.getISMRMRDKSpaceFilterFromName(*str);
        filterE1_sigma_ = this->get_double_value("filterE1_sigma");
        filterE1_width_ = this->get_double_value("filterE1_width");
        GADGET_CONDITION_MSG(verboseMode_, "filterE1_type_ is " << *str);
        GADGET_CONDITION_MSG(verboseMode_, "filterE1_sigma_ is " << filterE1_sigma_);
        GADGET_CONDITION_MSG(verboseMode_, "filterE1_width_ is " << filterE1_width_);

        str = this->get_string_value("filterE2");
        filterE2_type_ = gtPlus_util_.getISMRMRDKSpaceFilterFromName(*str);
        filterE2_sigma_ = this->get_double_value("filterE2_sigma");
        filterE2_width_ = this->get_double_value("filterE2_width");
        GADGET_CONDITION_MSG(verboseMode_, "filterE2_type_ is " << *str);
        GADGET_CONDITION_MSG(verboseMode_, "filterE2_sigma_ is " << filterE2_sigma_);
        GADGET_CONDITION_MSG(verboseMode_, "filterE2_width_ is " << filterE2_width_);

        str = this->get_string_value("filterRefRO");
        filterRO_ref_type_ = gtPlus_util_.getISMRMRDKSpaceFilterFromName(*str);
        filterRO_ref_sigma_ = this->get_double_value("filterRefRO_sigma");
        filterRO_ref_width_ = this->get_double_value("filterRefRO_width");
        GADGET_CONDITION_MSG(verboseMode_, "filterRO_ref_type_ is " << *str);
        GADGET_CONDITION_MSG(verboseMode_, "filterRO_ref_sigma_ is " << filterRO_ref_sigma_);
        GADGET_CONDITION_MSG(verboseMode_, "filterRO_ref_width_ is " << filterRO_ref_width_);

        str = this->get_string_value("filterRefE1");
        filterE1_ref_type_ = gtPlus_util_.getISMRMRDKSpaceFilterFromName(*str);
        filterE1_ref_sigma_ = this->get_double_value("filterRefE1_sigma");
        filterE1_ref_width_ = this->get_double_value("filterRefE1_width");
        GADGET_CONDITION_MSG(verboseMode_, "filterE1_ref_type_ is " << *str);
        GADGET_CONDITION_MSG(verboseMode_, "filterE1_ref_sigma_ is " << filterE1_ref_sigma_);
        GADGET_CONDITION_MSG(verboseMode_, "filterE1_ref_width_ is " << filterE1_ref_width_);

        str = this->get_string_value("filterRefE2");
        filterE2_ref_type_ = gtPlus_util_.getISMRMRDKSpaceFilterFromName(*str);
        filterE2_ref_sigma_ = this->get_double_value("filterRefE2_sigma");
        filterE2_ref_width_ = this->get_double_value("filterRefE2_width");
        GADGET_CONDITION_MSG(verboseMode_, "filterE2_ref_type_ is " << *str);
        GADGET_CONDITION_MSG(verboseMode_, "filterE2_ref_sigma_ is " << filterE2_ref_sigma_);
        GADGET_CONDITION_MSG(verboseMode_, "filterE2_ref_width_ is " << filterE2_ref_width_);

        str = this->get_string_value("filterPartialFourierRO");
        filterRO_pf_type_ = gtPlus_util_.getISMRMRDKSpaceFilterFromName(*str);
        filterRO_pf_sigma_ = this->get_double_value("filterPartialFourierRO_sigma");
        filterRO_pf_width_ = this->get_double_value("filterPartialFourierRO_width");
        filterRO_pf_densityComp_ = this->get_bool_value("filterPartialFourierRO_densityComp");
        GADGET_CONDITION_MSG(verboseMode_, "filterRO_pf_type_ is " << *str);
        GADGET_CONDITION_MSG(verboseMode_, "filterRO_pf_sigma_ is " << filterRO_pf_sigma_);
        GADGET_CONDITION_MSG(verboseMode_, "filterRO_pf_width_ is " << filterRO_pf_width_);
        GADGET_CONDITION_MSG(verboseMode_, "filterRO_pf_densityComp_ is " << filterRO_pf_densityComp_);

        str = this->get_string_value("filterPartialFourierE1");
        filterE1_pf_type_ = gtPlus_util_.getISMRMRDKSpaceFilterFromName(*str);
        filterE1_pf_sigma_ = this->get_double_value("filterPartialFourierE1_sigma");
        filterE1_pf_width_ = this->get_double_value("filterPartialFourierE1_width");
        filterE1_pf_densityComp_ = this->get_bool_value("filterPartialFourierE1_densityComp");
        GADGET_CONDITION_MSG(verboseMode_, "filterE1_pf_type_ is " << *str);
        GADGET_CONDITION_MSG(verboseMode_, "filterE1_pf_sigma_ is " << filterE1_pf_sigma_);
        GADGET_CONDITION_MSG(verboseMode_, "filterE1_pf_width_ is " << filterE1_pf_width_);
        GADGET_CONDITION_MSG(verboseMode_, "filterE1_pf_densityComp_ is " << filterE1_pf_densityComp_);

        str = this->get_string_value("filterPartialFourierE2");
        filterE2_pf_type_ = gtPlus_util_.getISMRMRDKSpaceFilterFromName(*str);
        filterE2_pf_sigma_ = this->get_double_value("filterPartialFourierE2_sigma");
        filterE2_pf_width_ = this->get_double_value("filterPartialFourierE2_width");
        filterE2_pf_densityComp_ = this->get_bool_value("filterPartialFourierE2_densityComp");
        GADGET_CONDITION_MSG(verboseMode_, "filterE2_pf_type_ is " << *str);
        GADGET_CONDITION_MSG(verboseMode_, "filterE2_pf_sigma_ is " << filterE2_pf_sigma_);
        GADGET_CONDITION_MSG(verboseMode_, "filterE2_pf_width_ is " << filterE2_pf_width_);
        GADGET_CONDITION_MSG(verboseMode_, "filterE2_pf_densityComp_ is " << filterE2_pf_densityComp_);

        GADGET_CONDITION_MSG(verboseMode_, "-----------------------------------------------");

        CloudComputing_ = this->get_bool_value("CloudComputing");
        CloudSize_ = (unsigned int)(this->get_int_value("CloudSize"));

        GADGET_CONDITION_MSG(verboseMode_, "CloudComputing_ is " << CloudComputing_);
        GADGET_CONDITION_MSG(verboseMode_, "CloudSize_ is " << CloudSize_);

        str = this->get_string_value("cloudNodeFile");
        cloud_node_file_ = *str;
        GADGET_CONDITION_MSG(verboseMode_, "cloud_node_file_ is " << cloud_node_file_);

        // read in the cloud information for every node
        gt_cloud_.resize(CloudSize_);

        for ( unsigned int ii=0; ii<CloudSize_; ii++ )
        {
            std::ostringstream ostreamstr1;
            ostreamstr1 << "CloudNode" << ii << "_IP" << std::ends;
            boost::shared_ptr<std::string> IP = this->get_string_value(ostreamstr1.str().c_str());
            gt_cloud_[ii].get<0>() = *IP;

            std::ostringstream ostreamstr2;
            ostreamstr2 << "CloudNode" << ii << "_Port" << std::ends;
            boost::shared_ptr<std::string> Port = this->get_string_value(ostreamstr2.str().c_str());
            gt_cloud_[ii].get<1>() = *Port;

            std::ostringstream ostreamstr3;
            ostreamstr3 << "CloudNode" << ii << "_XMLConfiguration" << std::ends;
            boost::shared_ptr<std::string> xmlName = this->get_string_value(ostreamstr3.str().c_str());
            gt_cloud_[ii].get<2>() = *xmlName;

            std::ostringstream ostreamstr4;
            ostreamstr4 << "CloudNode" << ii << "_ComputingPowerIndex" << std::ends;
            unsigned int computingPowerIndex = this->get_int_value(ostreamstr4.str().c_str());
            gt_cloud_[ii].get<3>() = computingPowerIndex;

            GADGET_CONDITION_MSG(verboseMode_, "Cloud Node " << ii << " : " << gt_cloud_[ii]);
        }

        GADGET_CONDITION_MSG(verboseMode_, "-----------------------------------------------");

        GADGET_CONDITION_MSG(verboseMode_, "==================================================================");

        GADGET_CONDITION_MSG(verboseMode_, "------> GtPlus recon parameters <------");

        workOrderPara_.upstream_coil_compression_ = this->get_bool_value("upstream_coil_compression");
        GADGET_CONDITION_MSG(verboseMode_, "upstream_coil_compression_ is " << workOrderPara_.upstream_coil_compression_);

        workOrderPara_.upstream_coil_compression_thres_ = this->get_double_value("upstream_coil_compression_thres");
        GADGET_CONDITION_MSG(verboseMode_, "upstream_coil_compression_thres_ is " << workOrderPara_.upstream_coil_compression_thres_);

        workOrderPara_.upstream_coil_compression_num_modesKept_ = this->get_int_value("upstream_coil_compression_num_modesKept");
        GADGET_CONDITION_MSG(verboseMode_, "upstream_coil_compression_num_modesKept_ is " << workOrderPara_.upstream_coil_compression_num_modesKept_);

        workOrderPara_.downstream_coil_compression_ = this->get_bool_value("downstream_coil_compression");
        GADGET_CONDITION_MSG(verboseMode_, "downstream_coil_compression_ is " << workOrderPara_.downstream_coil_compression_);

        workOrderPara_.coil_compression_thres_ = this->get_double_value("coil_compression_thres");
        GADGET_CONDITION_MSG(verboseMode_, "coil_compression_thres_ is " << workOrderPara_.coil_compression_thres_);

        workOrderPara_.coil_compression_num_modesKept_ = this->get_int_value("coil_compression_num_modesKept");
        GADGET_CONDITION_MSG(verboseMode_, "coil_compression_num_modesKept_ is " << workOrderPara_.coil_compression_num_modesKept_);

        GADGET_CONDITION_MSG(verboseMode_, "-----------------------------------------------");

        str = this->get_string_value("coil_map_algorithm");
        workOrderPara_.coil_map_algorithm_ = gtPlus_util_.getISMRMRDCoilMapAlgoFromName(*str);
        GADGET_CONDITION_MSG(verboseMode_, "coil_map_algorithm_ is " << *str);

        workOrderPara_.csm_kSize_ = (unsigned long long)(this->get_int_value("csm_kSize"));
        GADGET_CONDITION_MSG(verboseMode_, "csm_kSize_ is " << workOrderPara_.csm_kSize_);

        workOrderPara_.csm_powermethod_num_ = (unsigned long long)(this->get_int_value("csm_powermethod_num"));
        GADGET_CONDITION_MSG(verboseMode_, "csm_powermethod_num_ is " << workOrderPara_.csm_powermethod_num_);

        workOrderPara_.csm_iter_num_ = (unsigned long long)(this->get_int_value("csm_iter_num"));
        GADGET_CONDITION_MSG(verboseMode_, "csm_kSize_ is " << workOrderPara_.csm_iter_num_);

        workOrderPara_.csm_iter_thres_ = this->get_double_value("csm_iter_thres");
        GADGET_CONDITION_MSG(verboseMode_, "csm_iter_thres_ is " << workOrderPara_.csm_iter_thres_);

        GADGET_CONDITION_MSG(verboseMode_, "-----------------------------------------------");

        str = this->get_string_value("recon_algorithm");
        workOrderPara_.recon_algorithm_ = gtPlus_util_.getISMRMRDReconAlgoFromName(*str);
        GADGET_CONDITION_MSG(verboseMode_, "recon_algorithm_ is " << *str);

        workOrderPara_.recon_auto_parameters_ = this->get_bool_value("recon_auto_parameters");
        GADGET_CONDITION_MSG(verboseMode_, "recon_auto_parameters_ is " << workOrderPara_.recon_auto_parameters_);

        GADGET_CONDITION_MSG(verboseMode_, "-----------------------------------------------");

        workOrderPara_.grappa_kSize_RO_ = (unsigned long long)(this->get_int_value("grappa_kSize_RO"));
        workOrderPara_.grappa_kSize_E1_ = (unsigned long long)(this->get_int_value("grappa_kSize_E1"));
        workOrderPara_.grappa_kSize_E2_ = (unsigned long long)(this->get_int_value("grappa_kSize_E2"));
        workOrderPara_.grappa_reg_lamda_ = this->get_double_value("grappa_reg_lamda");

        GADGET_CONDITION_MSG(verboseMode_, "grappa_kSize_RO_ is " << workOrderPara_.grappa_kSize_RO_);
        GADGET_CONDITION_MSG(verboseMode_, "grappa_kSize_E1_ is " << workOrderPara_.grappa_kSize_E1_);
        GADGET_CONDITION_MSG(verboseMode_, "grappa_kSize_E2_ is " << workOrderPara_.grappa_kSize_E2_);
        GADGET_CONDITION_MSG(verboseMode_, "grappa_reg_lamda_ is " << workOrderPara_.grappa_reg_lamda_);

        GADGET_CONDITION_MSG(verboseMode_, "-----------------------------------------------");

        workOrderPara_.spirit_kSize_RO_ = (unsigned long long)(this->get_int_value("spirit_kSize_RO"));
        workOrderPara_.spirit_kSize_E1_ = (unsigned long long)(this->get_int_value("spirit_kSize_E1"));
        workOrderPara_.spirit_kSize_E2_ = (unsigned long long)(this->get_int_value("spirit_kSize_E2"));
        workOrderPara_.spirit_reg_lamda_ = this->get_double_value("spirit_reg_lamda");
        workOrderPara_.spirit_calib_over_determine_ratio_ = this->get_double_value("spirit_calib_over_determine_ratio");
        workOrderPara_.spirit_solve_symmetric_ = this->get_bool_value("spirit_solve_symmetric");
        workOrderPara_.spirit_iter_max_ = (unsigned long long)(this->get_int_value("spirit_iter_max"));
        workOrderPara_.spirit_iter_thres_ = this->get_double_value("spirit_iter_thres");
        workOrderPara_.spirit_print_iter_ = this->get_bool_value("spirit_print_iter");

        GADGET_CONDITION_MSG(verboseMode_, "spirit_kSize_RO_ is " << workOrderPara_.spirit_kSize_RO_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_kSize_E1_ is " << workOrderPara_.spirit_kSize_E1_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_kSize_E2_ is " << workOrderPara_.spirit_kSize_E2_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_reg_lamda_ is " << workOrderPara_.spirit_reg_lamda_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_calib_over_determine_ratio_ is " << workOrderPara_.spirit_calib_over_determine_ratio_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_solve_symmetric_ is " << workOrderPara_.spirit_solve_symmetric_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_iter_max_ is " << workOrderPara_.spirit_iter_max_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_iter_thres_ is " << workOrderPara_.spirit_iter_thres_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_print_iter_ is " << workOrderPara_.spirit_print_iter_);

        GADGET_CONDITION_MSG(verboseMode_, "-----------------------------------------------");

        workOrderPara_.spirit_perform_linear_ = this->get_bool_value("spirit_perform_linear");
        workOrderPara_.spirit_perform_nonlinear_ = this->get_bool_value("spirit_perform_nonlinear");
        workOrderPara_.spirit_parallel_imaging_lamda_ = this->get_double_value("spirit_parallel_imaging_lamda");
        workOrderPara_.spirit_image_reg_lamda_ = this->get_double_value("spirit_image_reg_lamda");
        workOrderPara_.spirit_data_fidelity_lamda_ = this->get_double_value("spirit_data_fidelity_lamda");
        workOrderPara_.spirit_ncg_iter_max_ = (unsigned long long)(this->get_int_value("spirit_ncg_iter_max"));
        workOrderPara_.spirit_ncg_iter_thres_ = this->get_double_value("spirit_ncg_iter_thres");
        workOrderPara_.spirit_ncg_print_iter_ = this->get_bool_value("spirit_ncg_print_iter");
        // spirit_ncg_scale_factor_ is computed from the data
        workOrderPara_.spirit_use_coil_sen_map_ = this->get_bool_value("spirit_use_coil_sen_map");
        workOrderPara_.spirit_use_moco_enhancement_ = this->get_bool_value("spirit_use_moco_enhancement");
        workOrderPara_.spirit_recon_moco_images_ = this->get_bool_value("spirit_recon_moco_images");
        workOrderPara_.spirit_RO_enhancement_ratio_ = this->get_double_value("spirit_RO_enhancement_ratio");
        workOrderPara_.spirit_E1_enhancement_ratio_ = this->get_double_value("spirit_E1_enhancement_ratio");
        workOrderPara_.spirit_E2_enhancement_ratio_ = this->get_double_value("spirit_E2_enhancement_ratio");
        workOrderPara_.spirit_temporal_enhancement_ratio_ = this->get_double_value("spirit_temporal_enhancement_ratio");
        workOrderPara_.spirit_2D_scale_per_chunk_ = this->get_bool_value("spirit_2D_scale_per_chunk");
        workOrderPara_.spirit_3D_scale_per_chunk_ = this->get_bool_value("spirit_3D_scale_per_chunk");

        GADGET_CONDITION_MSG(verboseMode_, "spirit_perform_linear_ is " << workOrderPara_.spirit_perform_linear_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_perform_nonlinear_ is " << workOrderPara_.spirit_perform_nonlinear_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_parallel_imaging_lamda_ is " << workOrderPara_.spirit_parallel_imaging_lamda_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_image_reg_lamda_ is " << workOrderPara_.spirit_image_reg_lamda_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_data_fidelity_lamda_ is " << workOrderPara_.spirit_data_fidelity_lamda_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_ncg_iter_max_ is " << workOrderPara_.spirit_ncg_iter_max_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_ncg_iter_thres_ is " << workOrderPara_.spirit_ncg_iter_thres_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_ncg_print_iter_ is " << workOrderPara_.spirit_ncg_print_iter_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_use_coil_sen_map_ is " << workOrderPara_.spirit_use_coil_sen_map_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_use_moco_enhancement_ is " << workOrderPara_.spirit_use_moco_enhancement_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_recon_moco_images_ is " << workOrderPara_.spirit_recon_moco_images_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_RO_enhancement_ratio_ is " << workOrderPara_.spirit_RO_enhancement_ratio_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_E1_enhancement_ratio_ is " << workOrderPara_.spirit_E1_enhancement_ratio_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_E2_enhancement_ratio_ is " << workOrderPara_.spirit_E2_enhancement_ratio_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_temporal_enhancement_ratio_ is " << workOrderPara_.spirit_temporal_enhancement_ratio_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_2D_scale_per_chunk_ is " << workOrderPara_.spirit_2D_scale_per_chunk_);
        GADGET_CONDITION_MSG(verboseMode_, "spirit_3D_scale_per_chunk_ is " << workOrderPara_.spirit_3D_scale_per_chunk_);

        GADGET_CONDITION_MSG(verboseMode_, "-----------------------------------------------");

        workOrderPara_.job_split_by_S_ = this->get_bool_value("job_split_by_S");
        workOrderPara_.job_num_of_N_ = (unsigned long long)(this->get_int_value("job_num_of_N"));
        workOrderPara_.job_max_Megabytes_ = (unsigned long long)(this->get_int_value("job_max_Megabytes"));
        workOrderPara_.job_overlap_ = (unsigned long long)(this->get_int_value("job_overlap"));
        workOrderPara_.job_perform_on_control_node_ = this->get_bool_value("job_perform_on_control_node");

        GADGET_CONDITION_MSG(verboseMode_, "job_split_by_S_ is " << workOrderPara_.job_split_by_S_);
        GADGET_CONDITION_MSG(verboseMode_, "job_num_of_N_ is " << workOrderPara_.job_num_of_N_);
        GADGET_CONDITION_MSG(verboseMode_, "job_max_Megabytes_ is " << workOrderPara_.job_max_Megabytes_);
        GADGET_CONDITION_MSG(verboseMode_, "job_overlap_ is " << workOrderPara_.job_overlap_);
        GADGET_CONDITION_MSG(verboseMode_, "job_perform_on_control_node_ is " << workOrderPara_.job_perform_on_control_node_);

        GADGET_CONDITION_MSG(verboseMode_, "-----------------------------------------------");

        str = this->get_string_value("partialFourier_algo");
        workOrderPara_.partialFourier_algo_ = gtPlus_util_.getISMRMRDPartialFourierReconAlgoFromName(*str);
        GADGET_CONDITION_MSG(verboseMode_, "partialFourier_algo_ is " << *str);

        workOrderPara_.partialFourier_homodyne_iters_ = (unsigned long long)(this->get_int_value("partialFourier_homodyne_iters"));
        workOrderPara_.partialFourier_homodyne_thres_ = this->get_double_value("partialFourier_homodyne_thres");
        workOrderPara_.partialFourier_homodyne_densityComp_ = this->get_bool_value("partialFourier_homodyne_densityComp");

        GADGET_CONDITION_MSG(verboseMode_, "partialFourier_homodyne_iters_ is " << workOrderPara_.partialFourier_homodyne_iters_);
        GADGET_CONDITION_MSG(verboseMode_, "partialFourier_homodyne_thres_ is " << workOrderPara_.partialFourier_homodyne_thres_);
        GADGET_CONDITION_MSG(verboseMode_, "partialFourier_homodyne_densityComp_ is " << workOrderPara_.partialFourier_homodyne_densityComp_);

        workOrderPara_.partialFourier_POCS_iters_ = (unsigned long long)(this->get_int_value("partialFourier_POCS_iters"));
        workOrderPara_.partialFourier_POCS_thres_ = this->get_double_value("partialFourier_POCS_thres");
        workOrderPara_.partialFourier_POCS_transitBand_ = (unsigned long long)(this->get_int_value("partialFourier_POCS_transitBand"));
        workOrderPara_.partialFourier_POCS_transitBand_E2_ = (unsigned long long)(this->get_int_value("partialFourier_POCS_transitBand_E2"));

        GADGET_CONDITION_MSG(verboseMode_, "partialFourier_POCS_iters_ is " << workOrderPara_.partialFourier_POCS_iters_);
        GADGET_CONDITION_MSG(verboseMode_, "partialFourier_POCS_thres_ is " << workOrderPara_.partialFourier_POCS_thres_);
        GADGET_CONDITION_MSG(verboseMode_, "partialFourier_POCS_transitBand_ is " << workOrderPara_.partialFourier_POCS_transitBand_);
        GADGET_CONDITION_MSG(verboseMode_, "partialFourier_POCS_transitBand_ is " << workOrderPara_.partialFourier_POCS_transitBand_E2_);

        workOrderPara_.partialFourier_FengHuang_kSize_RO_ = (unsigned long long)(this->get_int_value("partialFourier_FengHuang_kSize_RO"));
        workOrderPara_.partialFourier_FengHuang_kSize_E1_ = (unsigned long long)(this->get_int_value("partialFourier_FengHuang_kSize_E1"));
        workOrderPara_.partialFourier_FengHuang_kSize_E2_ = (unsigned long long)(this->get_int_value("partialFourier_FengHuang_kSize_E2"));
        workOrderPara_.partialFourier_FengHuang_thresReg_ = this->get_double_value("partialFourier_FengHuang_thresReg");
        workOrderPara_.partialFourier_FengHuang_sameKernel_allN_ = this->get_bool_value("partialFourier_FengHuang_sameKernel_allN");
        workOrderPara_.partialFourier_FengHuang_transitBand_ = (unsigned long long)(this->get_int_value("partialFourier_FengHuang_transitBand"));
        workOrderPara_.partialFourier_FengHuang_transitBand_E2_ = (unsigned long long)(this->get_int_value("partialFourier_FengHuang_transitBand_E2"));

        GADGET_CONDITION_MSG(verboseMode_, "partialFourier_FengHuang_kSize_RO_ is " << workOrderPara_.partialFourier_FengHuang_kSize_RO_);
        GADGET_CONDITION_MSG(verboseMode_, "partialFourier_FengHuang_kSize_E1_ is " << workOrderPara_.partialFourier_FengHuang_kSize_E1_);
        GADGET_CONDITION_MSG(verboseMode_, "partialFourier_FengHuang_kSize_E2_ is " << workOrderPara_.partialFourier_FengHuang_kSize_E2_);
        GADGET_CONDITION_MSG(verboseMode_, "partialFourier_FengHuang_thresReg_ is " << workOrderPara_.partialFourier_FengHuang_thresReg_);
        GADGET_CONDITION_MSG(verboseMode_, "partialFourier_FengHuang_sameKernel_allN_ is " << workOrderPara_.partialFourier_FengHuang_sameKernel_allN_);
        GADGET_CONDITION_MSG(verboseMode_, "partialFourier_FengHuang_transitBand_ is " << workOrderPara_.partialFourier_FengHuang_transitBand_);
        GADGET_CONDITION_MSG(verboseMode_, "partialFourier_FengHuang_transitBand_E2_ is " << workOrderPara_.partialFourier_FengHuang_transitBand_E2_);

        GADGET_CONDITION_MSG(verboseMode_, "-----------------------------------------------");

        recon_kspace_needed_ = this->get_bool_value("recon_kspace_needed");
        GADGET_CONDITION_MSG(verboseMode_, "recon_kspace_needed_ is " << recon_kspace_needed_);

        GADGET_CONDITION_MSG(verboseMode_, "-----------------------------------------------");
    }
    catch(...)
    {
        GADGET_ERROR_MSG("Errors in GtPlusReconGadget::readParameters() ... ");
        return false;
    }

    return true;
}

bool GtPlusReconGadget::parseGTCloudNodeFile(const std::string& filename, CloudType& gtCloud)
{
    std::string nodeFileName = ACE_OS::getenv("GADGETRON_HOME");
    nodeFileName.append("/config/gtCloud/");
    nodeFileName.append(filename);
    GADGET_CONDITION_MSG(verboseMode_, "Cloud node file name is " << nodeFileName);

    std::ifstream fs(nodeFileName.c_str(), std::ios::in);
    if (!fs.is_open()) 
    {
        GADGET_WARN_MSG("Cannot open GT CloudNodeFile; use the local setting instead ... ");
        return false;
    }

    // control node hostname
    std::string controlNode;
    fs >> controlNode;

    std::string portControlNode;
    fs >> portControlNode;

    // number of GadgetLevel nodes
    unsigned int num;
    fs >> num;

    gtCloud.resize(num);

    unsigned int n;
    for ( n=0; n<num; n++ )
    {
        std::string gadgetNode;
        fs >> gadgetNode;

        std::string portGadgetNode;
        fs >> portGadgetNode;

        std::string xmlGadgetNode;
        fs >> xmlGadgetNode;

        unsigned int computingPowerIndex;
        fs >> computingPowerIndex;

        gtCloud[n].get<0>() = gadgetNode;
        gtCloud[n].get<1>() = portGadgetNode;
        gtCloud[n].get<2>() = xmlGadgetNode;
        gtCloud[n].get<3>() = computingPowerIndex;

        GADGET_CONDITION_MSG(verboseMode_, "Gadget Node " << n << " : " << gt_cloud_[n]);
    }

    fs.close();

    return true;
}

int GtPlusReconGadget::process_config(ACE_Message_Block* mb)
{
    // [Ro E1 Cha Slice E2 Con Phase Rep Set Seg]
    //   0  1  2   3    4   5    6     7  8   9

    verboseMode_ = this->get_bool_value("verboseMode");

    // read parameters from xml
    image_series_ = this->get_int_value("image_series");

    // read in parameters from the xml
    GADGET_CHECK_RETURN(this->readParameters(), GADGET_FAIL);

    boost::shared_ptr<ISMRMRD::ismrmrdHeader> cfg = Gadgetron::parseIsmrmrdXMLHeader(std::string(mb->rd_ptr()));

    ISMRMRD::ismrmrdHeader::acquisitionSystemInformation_optional e_acq = cfg->acquisitionSystemInformation();
    num_acq_channels_ = e_acq->receiverChannels().get();
    GADGET_CONDITION_MSG(verboseMode_, "Number of acquisition channels : " << num_acq_channels_);

    ISMRMRD::ismrmrdHeader::encoding_sequence e_seq = cfg->encoding();
    if (e_seq.size() != 1)
    {
        GADGET_DEBUG2("Number of encoding spaces: %d\n", e_seq.size());
        GADGET_DEBUG1("This simple GtPlusReconGadget only supports one encoding space\n");
        return GADGET_FAIL;
    }

    // find out the encoding space 
    ISMRMRD::encodingSpaceType e_space = (*e_seq.begin()).encodedSpace();
    ISMRMRD::encodingSpaceType r_space = (*e_seq.begin()).reconSpace();
    ISMRMRD::encodingLimitsType e_limits = (*e_seq.begin()).encodingLimits();

    matrix_size_encoding_[0] = e_space.matrixSize().x();
    matrix_size_encoding_[1] = e_space.matrixSize().y();
    matrix_size_encoding_[2] = e_space.matrixSize().z();
    GADGET_CONDITION_MSG(verboseMode_, "Encoding matrix size: " << matrix_size_encoding_[0] << " " << matrix_size_encoding_[1] << " " << matrix_size_encoding_[2]);

    field_of_view_encoding_[0] = e_space.fieldOfView_mm().x();
    field_of_view_encoding_[1] = e_space.fieldOfView_mm().y();
    field_of_view_encoding_[2] = e_space.fieldOfView_mm().z();
    GADGET_CONDITION_MSG(verboseMode_, "Encoding field_of_view : " << field_of_view_encoding_[0] << " " << field_of_view_encoding_[1] << " " << field_of_view_encoding_[2]);

    // find the recon space
    matrix_size_recon_[0] = r_space.matrixSize().x();
    matrix_size_recon_[1] = r_space.matrixSize().y();
    matrix_size_recon_[2] = r_space.matrixSize().z();
    GADGET_CONDITION_MSG(verboseMode_, "Recon matrix size : " << matrix_size_recon_[0] << " " << matrix_size_recon_[1] << " " << matrix_size_recon_[2]);

    field_of_view_recon_[0] = r_space.fieldOfView_mm().x();
    field_of_view_recon_[1] = r_space.fieldOfView_mm().y();
    field_of_view_recon_[2] = r_space.fieldOfView_mm().z();
    GADGET_CONDITION_MSG(verboseMode_, "Recon field_of_view :  " << field_of_view_recon_[0] << " " << field_of_view_recon_[1] << " " << field_of_view_recon_[2]);

    reconE1_ = matrix_size_recon_[1];
    GADGET_CONDITION_MSG(verboseMode_, "reconE1_ is " << reconE1_);

    reconE2_ = matrix_size_recon_[2];
    GADGET_CONDITION_MSG(verboseMode_, "reconE2_ is " << reconE2_);

    kSpaceMaxAcqE1No_ = e_limits.kspace_encoding_step_1().get().maximum();
    GADGET_CONDITION_MSG(verboseMode_, "kSpaceMaxAcqE1No_ is " << kSpaceMaxAcqE1No_);

    kSpaceMaxAcqE2No_ = e_limits.kspace_encoding_step_2().get().maximum();
    GADGET_CONDITION_MSG(verboseMode_, "kSpaceMaxAcqE2No_ is " << kSpaceMaxAcqE2No_);

    aSpacing_[0] = field_of_view_recon_[0]/matrix_size_recon_[0];
    aSpacing_[1] = field_of_view_recon_[1]/reconE1_;
    aSpacing_[2] = field_of_view_recon_[2]/reconE2_;

    gt_exporter_.setPixelSize(aSpacing_[0], aSpacing_[1], aSpacing_[2], aSpacing_[3], aSpacing_[4], aSpacing_[5], aSpacing_[6]);

    // find the maximal encoding size
    if (e_limits.kspace_encoding_step_1().present()) 
    {
        meas_max_idx_.kspace_encode_step_1 = e_limits.kspace_encoding_step_1().get().maximum();
    }
    else
    {
        meas_max_idx_.kspace_encode_step_1 = 0;
        std::cout << "Setting number of kspace_encode_step_1 to 0" << std::endl;
        return GADGET_FAIL;
    }

    if (e_limits.set().present())
    {
        meas_max_idx_.set = e_limits.set().get().maximum() - 1;
        if ( meas_max_idx_.set < 0 ) meas_max_idx_.set = 0;
    }
    else
    {
        meas_max_idx_.set = 0;
    }

    if (e_limits.phase().present())
    {
        meas_max_idx_.phase = e_limits.phase().get().maximum()-1;
        if ( meas_max_idx_.phase < 0 ) meas_max_idx_.phase = 0;
    }
    else
    {
        meas_max_idx_.phase = 0;
    }

    if (e_limits.kspace_encoding_step_2().present())
    {
        meas_max_idx_.kspace_encode_step_2 = e_limits.kspace_encoding_step_2().get().maximum();
    }
    else
    {
        meas_max_idx_.kspace_encode_step_2 = 0;
    }

    if (e_limits.contrast().present())
    {
        meas_max_idx_.contrast = e_limits.contrast().get().maximum()-1;
        if ( meas_max_idx_.contrast < 0 ) meas_max_idx_.contrast = 0;
    }
    else
    {
        meas_max_idx_.contrast = 0;
    }

    if (e_limits.slice().present())
    {
        meas_max_idx_.slice = e_limits.slice().get().maximum();
    }
    else
    {
        meas_max_idx_.slice = 0;
    }

    if (e_limits.repetition().present())
    {
        meas_max_idx_.repetition = e_limits.repetition().get().maximum();
    }
    else
    {
        meas_max_idx_.repetition = 0;
    }

    if (e_limits.average().present())
    {
        meas_max_idx_.average = e_limits.average().get().maximum()-1;
    }
    else
    {
        meas_max_idx_.average = 0;
    }

    if (e_limits.segment().present())
    {
        // meas_max_idx_.segment = e_limits.segment().get().maximum()-1;
        meas_max_idx_.segment = 0;
    }
    else
    {
        meas_max_idx_.segment = 0;
    }

    // find out the PAT mode
    ISMRMRD::ismrmrdHeader::parallelImaging_optional p_imaging_type = cfg->parallelImaging();
    ISMRMRD::parallelImagingType p_imaging = *p_imaging_type;

    acceFactorE1_ = (long)(p_imaging.accelerationFactor().kspace_encoding_step_1());
    acceFactorE2_ = (long)(p_imaging.accelerationFactor().kspace_encoding_step_2());
    GADGET_CONDITION_MSG(verboseMode_, "acceFactorE1 is " << acceFactorE1_);
    GADGET_CONDITION_MSG(verboseMode_, "acceFactorE2 is " << acceFactorE2_);

    ISMRMRD::calibrationModeType::value calib = *(p_imaging.calibrationMode());

    bool separate_ = (calib == ISMRMRD::calibrationModeType::separate);
    bool embedded_ = (calib == ISMRMRD::calibrationModeType::embedded);
    bool interleaved_ = (calib == ISMRMRD::calibrationModeType::interleaved);
    bool other_ = (calib == ISMRMRD::calibrationModeType::other);

    if ( separate_ ) { GADGET_CONDITION_MSG(verboseMode_, "Colibration mode is separate"); }
    if ( embedded_ ) { GADGET_CONDITION_MSG(verboseMode_, "Colibration mode is embedded"); }
    if ( interleaved_ ) { GADGET_CONDITION_MSG(verboseMode_, "Colibration mode is interleaved"); }
    if ( other_ ) { GADGET_CONDITION_MSG(verboseMode_, "Colibration mode is other"); }

    if ( other_ && acceFactorE1_==1 && acceFactorE2_==1 )
    {
        GADGET_CONDITION_MSG(verboseMode_, "Colibration mode is changed to ISMRMRD_interleaved");
        CalibMode_ = Gadgetron::gtPlus::ISMRMRD_interleaved;
        acceFactorE1_ = 2;
    }

    if ( interleaved_ )
    {
        CalibMode_ = Gadgetron::gtPlus::ISMRMRD_interleaved;

        if ( p_imaging.interleavingDimension().present() )
        {
            if ( *(p_imaging.interleavingDimension()) == ISMRMRD::interleavingDimensionType::phase )
            {
                InterleaveDim_ = Gadgetron::gtPlus::DIM_Phase;
            }

            if ( *(p_imaging.interleavingDimension()) == ISMRMRD::interleavingDimensionType::repetition )
            {
                InterleaveDim_ = Gadgetron::gtPlus::DIM_Repetition;
            }

            if ( *(p_imaging.interleavingDimension()) == ISMRMRD::interleavingDimensionType::average )
            {
                InterleaveDim_ = Gadgetron::gtPlus::DIM_Average;
            }

            if ( *(p_imaging.interleavingDimension()) == ISMRMRD::interleavingDimensionType::contrast )
            {
                InterleaveDim_ = Gadgetron::gtPlus::DIM_Contrast;
            }

            if ( *(p_imaging.interleavingDimension()) == ISMRMRD::interleavingDimensionType::other )
            {
                InterleaveDim_ = Gadgetron::gtPlus::DIM_other1;
            }

            GADGET_CONDITION_MSG(verboseMode_, "InterleaveDim is " << gtPlus_util_.getISMRMRDDimName(InterleaveDim_));
        }
    }

    if ( embedded_ )
    {
        CalibMode_ = Gadgetron::gtPlus::ISMRMRD_embedded;
    }

    if ( separate_ )
    {
        CalibMode_ = Gadgetron::gtPlus::ISMRMRD_separate;
    }

    if ( calib == ISMRMRD::calibrationModeType::external )
    {
        CalibMode_ = Gadgetron::gtPlus::ISMRMRD_external;
    }

    if ( calib == ISMRMRD::calibrationModeType::other )
    {
        CalibMode_ = Gadgetron::gtPlus::ISMRMRD_other;
    }

    // generate the destination folder
    if ( !debugFolder_.empty() )
    {
        GADGET_CHECK_RETURN_FALSE(generateDebugFolderPath(debugFolder_, debugFolder_fullPath_));
    }
    else
    {
        GADGET_MSG("GtPlusRecon, debugFolder is not set ...");
    }

    if ( !debugFolder2_.empty() )
    {
        GADGET_CHECK_RETURN_FALSE(generateDebugFolderPath(debugFolder2_, debugFolder2_fullPath_));
    }
    else
    {
        GADGET_MSG("GtPlusRecon, debugFolder2 is not set ...");
    }

    return GADGET_OK;
}

bool GtPlusReconGadget::
generateDebugFolderPath(const std::string& debugFolder, std::string& debugFolderPath)
{
    debugFolderPath = ACE_OS::getenv("GADGETRON_HOME");
    debugFolderPath.append("/");
    debugFolderPath.append(debugFolder);
    debugFolderPath.append("/");
    GADGET_CONDITION_MSG(verboseMode_, "Debug folder is " << debugFolderPath);
    return true;
}

void GtPlusReconGadget::
getCurrentMoment(std::string& procTime)
{
    char timestamp[100];
    time_t mytime;
    struct tm *mytm;
    mytime=time(NULL);
    mytm=localtime(&mytime);
    strftime(timestamp, sizeof(timestamp),"_%a_%d_%b_%Y_%H_%M_%S",mytm);
    procTime = timestamp;
}

int GtPlusReconGadget::process(Gadgetron::GadgetContainerMessage< GtPlusGadgetImageArray >* m1, Gadgetron::GadgetContainerMessage< WorkOrderType > * m2)
{
    GADGET_CONDITION_MSG(verboseMode_, "GtPlusReconGadget::process(...) starts ... ");

    processed_called_times_++;

    GtPlusGadgetImageArray* images = m1->getObjectPtr();

    boost::shared_ptr< std::vector<unsigned long long> > dims = m2->getObjectPtr()->data_.get_dimensions();

    GADGET_CONDITION_MSG(verboseMode_, "[Ro E1 Cha Slice E2 Con Phase Rep Set Seg] = [" 
        << (*dims)[0] << " " << (*dims)[1] << " " << (*dims)[2] << " " << (*dims)[3] << " " << (*dims)[4] 
        << " " << (*dims)[5] << " " << (*dims)[6] << " " << (*dims)[7] << " " << (*dims)[8] << " " << (*dims)[9] << "]");

    dimensions_ = *dims;

    GADGET_CONDITION_MSG(verboseMode_, "GtPlusReconGadget::process(...) ends ... ");

    m1->release();
    return GADGET_OK;
}

int GtPlusReconGadget::computeSeriesImageNumber (ISMRMRD::ImageHeader& imheader, unsigned long long nCHA, unsigned long long cha, unsigned long long nE2, unsigned long long e2)
{
    int nSET = meas_max_idx_.set+1;
    int nREP = meas_max_idx_.repetition+1;
    int nPHS = meas_max_idx_.phase+1;
    int nSLC = meas_max_idx_.slice+1;
    int nCON = meas_max_idx_.contrast+1;
    if ( nE2 == 0 ) nE2 = 1;

    int imageNum = imheader.repetition*nSET*nPHS*nCON*nSLC*nE2*nCHA 
                    + imheader.set*nPHS*nCON*nSLC*nE2*nCHA 
                    + imheader.phase*nCON*nSLC*nE2*nCHA 
                    + imheader.contrast*nSLC*nE2*nCHA
                    + imheader.slice*nE2*nCHA 
                    + e2*nCHA 
                    + cha 
                    + 1;

    return imageNum;
}

bool GtPlusReconGadget::
addPrePostZeros(int centreNo, int sampleNo, int& PrePostZeros)
{
    // 1 : pre zeros
    // 2 : post zeros
    // 0 : no zeros
    PrePostZeros = 0;

    if ( sampleNo <= 1 )
        return true;

    if ( 2*centreNo == sampleNo )
    {
        PrePostZeros = 0;
    }

    if ( 2*centreNo < sampleNo )
    {
        PrePostZeros = 1;
    }

    if ( 2*centreNo > sampleNo )
    {
        PrePostZeros = 2;
    }

    return true;
}

bool GtPlusReconGadget::
scalingMagnitude(hoNDArray<float>& mag)
{
    if ( scalingFactor_ < 0 && !use_constant_scalingFactor_ )
    {
        // perform the scaling to [0 max_inten_value_]
        unsigned long long ind;
        float maxInten;

        unsigned long long RO = mag.get_size(0);
        unsigned long long E1 = mag.get_size(1);
        unsigned long long num = mag.get_number_of_elements()/(RO*E1);

        if ( num <= 24 )
        {
            GADGET_CHECK_RETURN_FALSE(Gadgetron::maxAbsolute(mag, maxInten, ind));
        }
        else
        {
            hoNDArray<float> magPartial(RO, E1, 24, mag.get_data_ptr()+(num/2 - 12)*RO*E1);
            GADGET_CHECK_RETURN_FALSE(Gadgetron::maxAbsolute(magPartial, maxInten, ind));
        }
        if ( maxInten < FLT_EPSILON ) maxInten = 1.0f;

        if ( (maxInten<min_intensity_value_) || (maxInten>max_intensity_value_) )
        {
            GADGET_CONDITION_MSG(verboseMode_, "Using the dynamic intensity scaling factor - may not have noise prewhitening performed ... ");
            scalingFactor_ = (float)(max_intensity_value_US_)/maxInten;
        }
        else
        {
            GADGET_CONDITION_MSG(verboseMode_, "Using the fixed intensity scaling factor - must have noise prewhitening performed ... ");
            scalingFactor_ = SNR_NOISEFLOOR_SCALEFACTOR;

            while ( (maxInten*scalingFactor_ > max_intensity_value_) && (scalingFactor_>=2) )
            {
                scalingFactor_ /= 2;
            }

            if (maxInten*scalingFactor_ > max_intensity_value_)
            {
                GADGET_CONDITION_MSG(verboseMode_, "The fixed intensity scaling factor leads to dynamic range overflow - switch to dyanmic intensity scaling ... ");
                scalingFactor_ = (float)(max_intensity_value_)/maxInten;
            }

            use_constant_scalingFactor_ = true;
        }

        GADGET_CONDITION_MSG(verboseMode_, "scalingFactor_ : " << scalingFactor_);
        GADGET_CHECK_RETURN_FALSE(scal((float)scalingFactor_, mag));
    }
    else
    {
        GADGET_CONDITION_MSG(verboseMode_, "Using the fixed intensity scaling factor - scaling factor has been preset to be : " << scalingFactor_ << " ... ");
        GADGET_CHECK_RETURN_FALSE(scal((float)scalingFactor_, mag));
    }

    return true;
}

bool GtPlusReconGadget::
generateKSpaceFilter(WorkOrderType& workOrder)
{
    try
    {
        unsigned long long RO = workOrder.data_.get_size(0);
        unsigned long long E1 = workOrder.data_.get_size(1);
        unsigned long long E2 = workOrder.data_.get_size(4);

        unsigned long long RO_ref = workOrder.ref_.get_size(0);
        unsigned long long E1_ref = workOrder.ref_.get_size(1);
        unsigned long long E2_ref = workOrder.ref_.get_size(4);

        if ( workOrder.CalibMode_ == Gadgetron::gtPlus::ISMRMRD_interleaved )
        {
            RO_ref = RO;
            E1_ref = E1;
            E2_ref = E2;
        }

        // image data filter
        if ( RO>1 && filterRO_type_ != ISMRMRD_FILTER_NONE )
        {
            workOrder.filterRO_.create(RO);
            GADGET_CHECK_RETURN_FALSE(gtPlus_util_.generateSymmetricFilter(RO, workOrder.filterRO_, filterRO_type_, filterRO_sigma_, std::ceil(filterRO_width_*RO)));
            GADGET_EXPORT_ARRAY_COMPLEX(debugFolder_fullPath_, gt_exporter_, workOrder.filterRO_, "filterRO");
        }

        if ( E1>1 && filterE1_type_ != ISMRMRD_FILTER_NONE )
        {
            workOrder.filterE1_.create(E1);
            GADGET_CHECK_RETURN_FALSE(gtPlus_util_.generateSymmetricFilter(E1, workOrder.filterE1_, filterE1_type_, filterE1_sigma_, std::ceil(filterE1_width_*E1)));
            GADGET_EXPORT_ARRAY_COMPLEX(debugFolder_fullPath_, gt_exporter_, workOrder.filterE1_, "filterE1");
        }

        if ( E2>1 && filterE2_type_ != ISMRMRD_FILTER_NONE )
        {
            workOrder.filterE2_.create(E2);
            GADGET_CHECK_RETURN_FALSE(gtPlus_util_.generateSymmetricFilter(E2, workOrder.filterE2_, filterE2_type_, filterE2_sigma_, std::ceil(filterE2_width_*E2)));
            GADGET_EXPORT_ARRAY_COMPLEX(debugFolder_fullPath_, gt_exporter_, workOrder.filterE2_, "filterE2");
        }

        // ref data filter
        if ( workOrder.ref_.get_number_of_elements() > 0 )
        {
            unsigned long long startRO(0), endRO(0), startE1(0), endE1(0), startE2(0), endE2(0);
            if ( E2_ref == 1 )
            {
                GADGET_CHECK_RETURN_FALSE(gtPlus_util_complex_.detectSampledRegion2D(workOrder.ref_, startRO, endRO, startE1, endE1));
            }
            else
            {
                GADGET_CHECK_RETURN_FALSE(gtPlus_util_complex_.detectSampledRegion3D(workOrder.ref_, startRO, endRO, startE1, endE1, startE2, endE2));
            }

            if ( (workOrder.CalibMode_ == ISMRMRD_interleaved) || (workOrder.CalibMode_ == ISMRMRD_embedded) )
            {
                // use the image data sample range
                startRO = workOrder.start_RO_; if ( startRO < 0 ) startRO=0;
                endRO = workOrder.end_RO_; if ( endRO < 0 ) endRO = RO_ref-1;
            }

            if ( RO_ref > 1 && filterRO_ref_type_ != ISMRMRD_FILTER_NONE )
            {
                workOrder.filterRO_ref_.create(RO_ref);
                GADGET_CHECK_RETURN_FALSE(gtPlus_util_.generateSymmetricFilterForRef(RO_ref, startRO, endRO, workOrder.filterRO_ref_, filterRO_ref_type_, filterRO_ref_sigma_, std::ceil(filterRO_ref_width_*RO_ref)));
                GADGET_EXPORT_ARRAY_COMPLEX(debugFolder_fullPath_, gt_exporter_, workOrder.filterRO_ref_, "filterRO_ref");
            }

            if ( (workOrder.CalibMode_ == ISMRMRD_separate) || (workOrder.CalibMode_ == ISMRMRD_external) )
            {
                if ( E1_ref > 1 && filterE1_ref_type_ != ISMRMRD_FILTER_NONE )
                {
                    unsigned long long len = endE1-startE1+1;
                    workOrder.filterE1_ref_.create(len);
                    GADGET_CHECK_RETURN_FALSE(gtPlus_util_.generateSymmetricFilter(len, workOrder.filterE1_ref_, filterE1_ref_type_, filterE1_ref_sigma_, std::ceil(filterE1_ref_width_*len)));
                    GADGET_EXPORT_ARRAY_COMPLEX(debugFolder_fullPath_, gt_exporter_, workOrder.filterE1_ref_, "filterE1_ref");
                }

                if ( E2_ref > 1 && filterE2_ref_type_ != ISMRMRD_FILTER_NONE )
                {
                    unsigned long long len = endE2-startE2+1;
                    workOrder.filterE2_ref_.create(len);
                    GADGET_CHECK_RETURN_FALSE(gtPlus_util_.generateSymmetricFilter(len, workOrder.filterE2_ref_, filterE2_ref_type_, filterE2_ref_sigma_, std::ceil(filterE2_ref_width_*len)));
                    GADGET_EXPORT_ARRAY_COMPLEX(debugFolder_fullPath_, gt_exporter_, workOrder.filterE2_ref_, "filterE2_ref");
                }
            }
            else
            {
                // this makes sure for interleaved and embedded, the kspace filter is applied at correct lines
                if ( E1_ref > 1 && filterE1_ref_type_ != ISMRMRD_FILTER_NONE )
                {
                    unsigned long long len = E1_ref;
                    workOrder.filterE1_ref_.create(len);
                    GADGET_CHECK_RETURN_FALSE(gtPlus_util_.generateSymmetricFilterForRef(len, startE1, endE1, workOrder.filterE1_ref_, filterE1_ref_type_, filterE1_ref_sigma_, std::ceil(filterE1_ref_width_*len)));
                    GADGET_EXPORT_ARRAY_COMPLEX(debugFolder_fullPath_, gt_exporter_, workOrder.filterE1_ref_, "filterE1_ref");
                }

                if ( E2_ref > 1 && filterE2_ref_type_ != ISMRMRD_FILTER_NONE )
                {
                    unsigned long long len = E2_ref;
                    workOrder.filterE2_ref_.create(len);
                    GADGET_CHECK_RETURN_FALSE(gtPlus_util_.generateSymmetricFilterForRef(len, startE2, endE2, workOrder.filterE2_ref_, filterE2_ref_type_, filterE2_ref_sigma_, std::ceil(filterE2_ref_width_*len)));
                    GADGET_EXPORT_ARRAY_COMPLEX(debugFolder_fullPath_, gt_exporter_, workOrder.filterE2_ref_, "filterE2_ref");
                }
            }
        }

        // partial fourier handling filter
        if ( RO>1 && workOrder.start_RO_>=0 && workOrder.end_RO_>0 )
        {
            workOrder.filterRO_partialfourier_.create(RO);
            GADGET_CHECK_RETURN_FALSE(gtPlus_util_.generateAsymmetricFilter(RO, workOrder.start_RO_, workOrder.end_RO_, workOrder.filterRO_partialfourier_, filterRO_pf_type_, std::ceil(filterRO_pf_width_*RO), filterRO_pf_densityComp_));
            GADGET_EXPORT_ARRAY_COMPLEX(debugFolder_fullPath_, gt_exporter_, workOrder.filterRO_partialfourier_, "filterRO_partialfourier");
        }

        if ( E1>1 && workOrder.start_E1_>=0 && workOrder.end_E1_>0 )
        {
            workOrder.filterE1_partialfourier_.create(E1);
            GADGET_CHECK_RETURN_FALSE(gtPlus_util_.generateAsymmetricFilter(E1, workOrder.start_E1_, workOrder.end_E1_, workOrder.filterE1_partialfourier_, filterE1_pf_type_, std::ceil(filterE1_pf_width_*E1), filterE1_pf_densityComp_));
            GADGET_EXPORT_ARRAY_COMPLEX(debugFolder_fullPath_, gt_exporter_, workOrder.filterE1_partialfourier_, "filterE1_partialfourier");
        }

        if ( E2>1 && workOrder.start_E2_>=0 && workOrder.end_E2_>0 )
        {
            workOrder.filterE2_partialfourier_.create(E2);
            GADGET_CHECK_RETURN_FALSE(gtPlus_util_.generateAsymmetricFilter(E2, workOrder.start_E2_, workOrder.end_E2_, workOrder.filterE2_partialfourier_, filterE2_pf_type_, std::ceil(filterE2_pf_width_*E2), filterE2_pf_densityComp_));
            GADGET_EXPORT_ARRAY_COMPLEX(debugFolder_fullPath_, gt_exporter_, workOrder.filterE2_partialfourier_, "filterE2_partialfourier");
        }
    }
    catch(...)
    {
        GADGET_ERROR_MSG("Errors in GtPlusReconGadget::generateKSpaceFilter(...) ... ");
        return false;
    }

    return true;
}

bool GtPlusReconGadget::
recomputeImageGeometry(GtPlusGadgetImageArray* images, GtPlusGadgetImageExt& imageHeader, int slc, int e2, int con, int phs, int rep, int set, int seg, int maxE2)
{
    unsigned long long E2 = images->matrix_size[4];

    // if FOV are the same, return the stored image header, take care of E2 resizing
    //if ( GT_ABS(field_of_view_recon_[2]-field_of_view_encoding_[2])<0.1 )
    //{
    //    if ( maxE2 == E2 ) // no E2 resizing
    //    {
    //        int offset = images->get_offset(slc, e2, con, phs, rep, set, 0);
    //        imageHeader = images->imageArray_[offset];
    //    }
    //    else
    //    {
    //        double e2_sampled = e2*E2/(double)maxE2;

    //        unsigned long long e2_lower = std::floor(e2_sampled);
    //        if ( e2_lower >= E2 ) e2_lower = E2-1;

    //        unsigned long long e2_higher = std::ceil(e2_sampled);
    //        if ( e2_higher >= E2 ) e2_higher = E2-1;

    //        GtPlusGadgetImageExt imageHeader_lower, imageHeader_higher;

    //        if ( e2_lower == e2_higher )
    //        {
    //            int offset = images->get_offset(slc, e2_lower, con, phs, rep, set, 0);
    //            imageHeader.copy(images->imageArray_[offset]);
    //        }
    //        else
    //        {
    //            int offset_lower = images->get_offset(slc, e2_lower, con, phs, rep, set, 0);
    //            imageHeader_lower.copy(images->imageArray_[offset_lower]);

    //            int offset_higher = images->get_offset(slc, e2_higher, con, phs, rep, set, 0);
    //            imageHeader_higher.copy(images->imageArray_[offset_higher]);

    //            imageHeader = imageHeader_lower;
    //            imageHeader.recomputeHeader(imageHeader_higher, e2_higher-e2_sampled);
    //        }
    //    }
    //}
    //else
    //{
        // need to recompute image geometry
        // no need to consider RO and E1, because image position vector points to the image center

        if ( e2 >= E2 ) e2 = E2/2;

        int offsetCurr = images->get_offset(slc, e2, con, phs, rep, set, 0);
        imageHeader = images->imageArray_[offsetCurr];

        // find the center partition
        if ( E2 > 1 )
        {
            int midE2 = E2/2;
            int offset = images->get_offset(slc, midE2, con, phs, rep, set, 0);

            while ( GT_ABS(imageHeader.slice_dir[0])<1e-6 && GT_ABS(imageHeader.slice_dir[1])<1e-6 && GT_ABS(imageHeader.slice_dir[2])<1e-6 )
            {
                imageHeader = images->imageArray_[offset];
                midE2++;
                offset = images->get_offset(slc, midE2, con, phs, rep, set, 0);
            }

            // position vector for the center partition
            float posVec[3];
            posVec[0] = imageHeader.position[0];
            posVec[1] = imageHeader.position[1];
            posVec[2] = imageHeader.position[2];

            // slice direction
            float sliceVec[3];
            sliceVec[0] = imageHeader.slice_dir[0];
            sliceVec[1] = imageHeader.slice_dir[1];
            sliceVec[2] = imageHeader.slice_dir[2];

            midE2 = E2/2;

            // comput slice postion vector for this partition
            float posVecCurr[3];
            posVecCurr[0] = posVec[0] + aSpacing_[2]*sliceVec[0]*(e2-midE2+0.5);
            posVecCurr[1] = posVec[1] + aSpacing_[2]*sliceVec[1]*(e2-midE2+0.5);
            posVecCurr[2] = posVec[2] + aSpacing_[2]*sliceVec[2]*(e2-midE2+0.5);

            imageHeader.position[0] = posVecCurr[0];
            imageHeader.position[1] = posVecCurr[1];
            imageHeader.position[2] = posVecCurr[2];

            GADGET_CONDITION_MSG(verboseMode_, "--> image position : [" << imageHeader.position[0] << " , " << imageHeader.position[1] << " , " << imageHeader.position[2] << "]");

            imageHeader.field_of_view[2] = aSpacing_[2];

            imageHeader.user_int[0] = e2;
        }

        if ( imageHeader.measurement_uid == 0 )
        {
            GADGET_WARN_MSG("imageHeader.measurement_uid == 0");
        }

        /*double e2_sampled = 0;
        double coord_in_encoding_space = field_of_view_recon_[2]*e2/maxE2 + (field_of_view_encoding_[2]/2-field_of_view_recon_[2]/2);
        e2_sampled = E2 * coord_in_encoding_space/field_of_view_encoding_[2];

        if ( e2_sampled < 0 )
        {
            int offset = images->get_offset(slc, 0, con, phs, rep, set, 0);
            imageHeader.copy(images->imageArray_[offset]);
        }
        else if ( e2_sampled > E2-1 )
        {
            int offset = images->get_offset(slc, E2-1, con, phs, rep, set, 0);
            imageHeader.copy(images->imageArray_[offset]);
        }
        else
        {
            unsigned long long e2_lower = std::floor(e2_sampled);
            if ( e2_lower >= E2 ) e2_lower = E2-1;

            unsigned long long e2_higher = std::ceil(e2_sampled);
            if ( e2_higher >= E2 ) e2_higher = E2-1;

            GtPlusGadgetImageExt imageHeader_lower, imageHeader_higher;

            if ( e2_lower == e2_higher )
            {
                int offset = images->get_offset(slc, e2_lower, con, phs, rep, set, 0);
                imageHeader.copy(images->imageArray_[offset]);
            }
            else
            {
                int offset_lower = images->get_offset(slc, e2_lower, con, phs, rep, set, 0);
                imageHeader_lower.copy(images->imageArray_[offset_lower]);

                int offset_higher = images->get_offset(slc, e2_higher, con, phs, rep, set, 0);
                imageHeader_higher.copy(images->imageArray_[offset_higher]);

                imageHeader.copy(imageHeader_lower);
                imageHeader.recomputeHeader(imageHeader_higher, e2_higher-e2_sampled);
            }
        }*/
    //}

    return true;
}

bool GtPlusReconGadget::
sendOutReconMag(GtPlusGadgetImageArray* images, const hoNDArray<float>& res, int seriesNum, const std::vector<DimensionRecordType>& dimStartingIndexes, const std::string& prefix)
{
    try
    {
        boost::shared_ptr< std::vector<unsigned long long> > dims = res.get_dimensions();
        unsigned long long RO = (*dims)[0];
        unsigned long long E1 = (*dims)[1];
        unsigned long long CHA = (*dims)[2];
        unsigned long long SLC = (*dims)[3];
        unsigned long long E2 = (*dims)[4];
        unsigned long long CON = (*dims)[5];
        unsigned long long PHS = (*dims)[6];
        unsigned long long REP = (*dims)[7];
        unsigned long long SET = (*dims)[8];

        GADGET_CONDITION_MSG(true, "sending out images, acquisition boundary [RO E1 CHA SLC E2 CON PHS REP SET] = [" 
                                                                      << RO << " " << E1 << " " << CHA << " " 
                                                                      << SLC << " " << E2 << " " << CON << " " 
                                                                      << PHS << " " << REP << " " << SET << "] " );

        unsigned long long set(0), rep(0), phs(0), con(0), e2(0), slc(0), cha(0), seg(0);
        // unsigned long long set_sInd(0), rep_sInd(0), phs_sInd(0), con_sInd(0), e2_sInd(0), slc_sInd(0);

        for ( set=0; set<SET; set++ )
        {
            // GADGET_CHECK_RETURN_FALSE(gtPlus_util_.findDimIndex(dimStartingIndexes, DIM_Set, set_sInd));

            for ( rep=0; rep<REP; rep++ )
            {
                // GADGET_CHECK_RETURN_FALSE(gtPlus_util_.findDimIndex(dimStartingIndexes, DIM_Repetition, rep_sInd));

                for ( phs=0; phs<PHS; phs++ )
                {
                    // GADGET_CHECK_RETURN_FALSE(gtPlus_util_.findDimIndex(dimStartingIndexes, DIM_Phase, phs_sInd));

                    for ( con=0; con<CON; con++ )
                    {
                        // GADGET_CHECK_RETURN_FALSE(gtPlus_util_.findDimIndex(dimStartingIndexes, DIM_Contrast, con_sInd));

                        for ( e2=0; e2<E2; e2++ )
                        {
                            // GADGET_CHECK_RETURN_FALSE(gtPlus_util_.findDimIndex(dimStartingIndexes, DIM_Encoding2, e2_sInd));

                            for ( slc=0; slc<SLC; slc++ )
                            {
                                // GADGET_CHECK_RETURN_FALSE(gtPlus_util_.findDimIndex(dimStartingIndexes, DIM_Slice, slc_sInd));

                                GtPlusGadgetImageExt imageHeaderSent;
                                GADGET_CHECK_RETURN_FALSE(recomputeImageGeometry(images, imageHeaderSent, slc, e2, con, phs, rep, set, 0, E2));

                                //int offset = images->get_offset(slc, e2, con, phs, rep, set, 0);
                                //imageHeaderSent = images->imageArray_[offset];

                                if ( imageHeaderSent.measurement_uid == 0 )
                                {
                                    continue;
                                }

                                for ( cha=0; cha<CHA; cha++ )
                                {
                                    Gadgetron::GadgetContainerMessage<ISMRMRD::ImageHeader>* cm1 = new Gadgetron::GadgetContainerMessage<ISMRMRD::ImageHeader>();
                                    *cm1->getObjectPtr() = imageHeaderSent;

                                    cm1->getObjectPtr()->flags = 0;
                                    cm1->getObjectPtr()->image_data_type = ISMRMRD::DATA_FLOAT;
                                    cm1->getObjectPtr()->image_type = ISMRMRD::TYPE_MAGNITUDE;

                                    // image number and image series
                                    cm1->getObjectPtr()->image_index = computeSeriesImageNumber (*cm1->getObjectPtr(), CHA, cha, E2, e2);
                                    cm1->getObjectPtr()->image_series_index = seriesNum;
                                    // GADGET_CONDITION_MSG(verboseMode_, "image number " << cm1->getObjectPtr()->image_index << "    image series " << cm1->getObjectPtr()->image_series_index << " ... ");

                                    // set the time stamp
                                    // the time stamp of the first readout line in this 2D kspace is used

                                    Gadgetron::GadgetContainerMessage< Gadgetron::hoNDArray<float> >* cm2 = new Gadgetron::GadgetContainerMessage< Gadgetron::hoNDArray<float> >();
                                    cm1->cont(cm2);

                                    std::vector<unsigned long long> img_dims(2);
                                    img_dims[0] = RO;
                                    img_dims[1] = E1;

                                    //Fixing array dimensions (MSH)
                                    cm1->getObjectPtr()->matrix_size[0] = RO;
                                    cm1->getObjectPtr()->matrix_size[1] = E1;
                                    cm1->getObjectPtr()->matrix_size[2] = 1;
                                    cm1->getObjectPtr()->channels = 1;

                                    try
                                    {
                                        cm2->getObjectPtr()->create(&img_dims);
                                        Gadgetron::clear(cm2->getObjectPtr());
                                    }
                                    catch(...)
                                    {
                                        GADGET_DEBUG1("Unable to allocate new image\n");
                                        cm1->release();
                                        return false;
                                    }

                                    std::vector<unsigned long long> ind(9, 0);
                                    ind[2] = cha;
                                    ind[3] = slc;
                                    ind[4] = e2;
                                    ind[5] = con;
                                    ind[6] = phs;
                                    ind[7] = rep;
                                    ind[8] = set;

                                    memcpy(cm2->getObjectPtr()->begin(), res.begin()+res.calculate_offset(ind), sizeof(float)*RO*E1);

                                    if ( !debugFolder2_fullPath_.empty() )
                                    {
                                        std::ostringstream ostr;
                                        ostr << prefix << "_" << cm1->getObjectPtr()->image_index;
                                        GADGET_EXPORT_ARRAY(debugFolder2_fullPath_, gt_exporter_, *cm2->getObjectPtr(), ostr.str());

                                        //hoNDArray<unsigned short> imageUS2D;
                                        //imageUS2D.copyFrom(*cm2->getObjectPtr());
                                        //std::ostringstream ostr2;
                                        //ostr2 << prefix << "_US_" << cm1->getObjectPtr()->image_index;
                                        //GADGET_EXPORT_ARRAY(debugFolder2_fullPath_, gt_exporter_, imageUS2D, ostr2.str());
                                    }

                                    GADGET_CONDITION_MSG(true, "sending out 2D image [CHA SLC E2 CON PHS REP SET] = [" 
                                                                      << cha << " " 
                                                                      << cm1->getObjectPtr()->slice << " " 
                                                                      << e2 << " " 
                                                                      << cm1->getObjectPtr()->contrast << " " 
                                                                      << cm1->getObjectPtr()->phase << " " 
                                                                      << cm1->getObjectPtr()->repetition << " " 
                                                                      << cm1->getObjectPtr()->set << "] \t" 
                                                                      << " -- Image number -- " << cm1->getObjectPtr()->image_index);

                                    // send out the images
                                    if (this->next()->putq(cm1) < 0) 
                                    {
                                        return false;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    catch(...)
    {
        GADGET_ERROR_MSG("Errors in GtPlusReconGadget::sendOutReconMag(float) ... ");
        return false;
    }

    return true;
}

bool GtPlusReconGadget::
sendOutRecon(GtPlusGadgetImageArray* images, const hoNDArray<ValueType>& res, int seriesNum, const std::vector<DimensionRecordType>& dimStartingIndexes, const std::string& prefix)
{
    try
    {
        hoNDArray<float> mag(res.get_dimensions());
        GADGET_CHECK_RETURN_FALSE(Gadgetron::absolute(res, mag));
        GADGET_CHECK_RETURN_FALSE(scalingMagnitude(mag));
        GADGET_CHECK_RETURN_FALSE(this->sendOutReconMag(images, mag, seriesNum, dimStartingIndexes, prefix));
    }
    catch(...)
    {
        GADGET_ERROR_MSG("Errors in GtPlusReconGadget::sendOutRecon(ValueType) ... ");
        return false;
    }

    return true;
}

bool GtPlusReconGadget::sendOutRecon2D(GtPlusGadgetImageArray* images, const hoNDArray<ValueType>& res, int seriesNum, int imageNum)
{
    try
    {
        // extract the magnitude
        hoNDArray<float> mag(res.get_dimensions());
        GADGET_CHECK_RETURN_FALSE(Gadgetron::absolute(res, mag));
        GADGET_CHECK_RETURN_FALSE(scalingMagnitude(mag));
        GADGET_CHECK_RETURN_FALSE(sendOutRecon2D(images, mag, seriesNum, imageNum));
    }
    catch(...)
    {
        GADGET_ERROR_MSG("Exceptions happened in GtPlusReconGadget::sendOutRecon2D(...) ... ");
        return false;
    }

    return true;
}

bool GtPlusReconGadget::sendOutRecon2D(GtPlusGadgetImageArray* images, const hoNDArray<float>& res, int seriesNum, int imageNum)
{
    try
    {
        Gadgetron::GadgetContainerMessage<ISMRMRD::ImageHeader>* cm1 = new Gadgetron::GadgetContainerMessage<ISMRMRD::ImageHeader>();
        *cm1->getObjectPtr() = images->imageArray_[0];

        cm1->getObjectPtr()->flags = 0;
        cm1->getObjectPtr()->image_data_type = ISMRMRD::DATA_FLOAT;
        cm1->getObjectPtr()->image_type = ISMRMRD::TYPE_MAGNITUDE;

        // image number and image series
        cm1->getObjectPtr()->image_index = imageNum;
        cm1->getObjectPtr()->image_series_index = seriesNum;

        Gadgetron::GadgetContainerMessage< Gadgetron::hoNDArray<float> >* cm2 = new Gadgetron::GadgetContainerMessage< Gadgetron::hoNDArray<float> >();
        cm1->cont(cm2);

        std::vector<unsigned long long> img_dims(2);
        img_dims[0] = res.get_size(0);
        img_dims[1] = res.get_size(1);

        //Fixing array dimensions (MSH)
        cm1->getObjectPtr()->matrix_size[0] = res.get_size(0);
        cm1->getObjectPtr()->matrix_size[1] = res.get_size(1);
        cm1->getObjectPtr()->matrix_size[2] = 1;
        cm1->getObjectPtr()->channels = 1;

        try
        {
            cm2->getObjectPtr()->create(&img_dims);
        }
        catch(...)
        {
            GADGET_DEBUG1("Unable to allocate new image\n");
            cm1->release();
            return false;
        }

        memcpy(cm2->getObjectPtr()->begin(), res.begin(), sizeof(float)*res.get_size(0)*res.get_size(1));

        if ( !debugFolder2_fullPath_.empty() )
        {
            std::ostringstream ostr;
            ostr << "SentImage2D" << "_" << cm1->getObjectPtr()->image_index;
            GADGET_EXPORT_ARRAY(debugFolder2_fullPath_, gt_exporter_, *cm2->getObjectPtr(), ostr.str());
        }

        // send out the images
        if (this->next()->putq(cm1) < 0) 
        {
            return false;
        }
    }
    catch(...)
    {
        GADGET_ERROR_MSG("Errors in GtPlusReconGadget::sendOutRecon2D(float) ... ");
        return false;
    }

    return true;
}

GADGET_FACTORY_DECLARE(GtPlusReconGadget)

}
