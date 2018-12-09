#include "stdafx.h"

#include "enc.h"
#include "cvutils.h"

#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/ml.hpp>


//samples: NxD
//return value:   
//      miu:KxD
//      pi:1xK
//      sigma:DxD x K (converted to KxD where each line consists of one diagonal)
//      gamma:NxK
bool gmm_em(
	cv::Mat &samples,
	unsigned int k,       // number of components
	cv::Mat &miu,
	cv::Mat &pi,
	cv::Mat &sigma,
	cv::Mat &gamma)
{
	namespace ml = cv::ml;

	cv::Ptr<ml::EM> em = ml::EM::create();
	em->setClustersNumber(k);
	em->setCovarianceMatrixType(ml::EM::COV_MAT_DIAGONAL);
	em->setTermCriteria({ 2/*COUNT=1, EPS=2*/, 10/*maxcount*/, 1e-12/*eps*/ });

	bool ret = em->trainEM(samples, cv::noArray(), cv::noArray(), gamma);
	if (!ret) return false;

	std::vector<cv::Mat> sigmavec;

	miu = em->getMeans();
	pi = em->getWeights();
	em->getCovs(sigmavec);

	sigma.create(k, samples.cols, CV_32FC1);
	for (int i = 0; i < sigmavec.size(); i++) {
		cv::Mat diag;
		cv::transpose(sigmavec[i].diag(), diag);
		diag.copyTo(sigma.row(i));
	}


	return true;
}



// return value: 
//     adapted_miu, adapted_pi, adapted_sigma 
//     (all converted to CV_32FC1)
std::tuple<cv::Mat, cv::Mat, cv::Mat> gmm_adapt(
	const cv::Mat &samples,
	float r,          // relevance factor
	const cv::Mat &miu,     
	const cv::Mat &pi,      
	const cv::Mat &sigma,   
	const cv::Mat &gamma)
{
	// miu:KxD
	// pi:1xK
	// sigma:DxD x K (converted to KxD where each line consists of one diagonal)
	// gamma:NxK


	cv::Mat samples_32f = convertto32f(samples, false);       // the following codes will never modify samples_32f.
	cv::Mat miu_32f = convertto32f(miu, true);
	cv::Mat pi_32f = convertto32f(pi, true);
	cv::Mat sigma_32f = convertto32f(sigma, true);
	cv::Mat gamma_32f = convertto32f(gamma, false);           // the following codes will never modify gamma_32f.

	int N = gamma_32f.rows;
	int K = gamma_32f.cols;
	int D = miu_32f.cols;

	cv::Mat tmp;

	// nk:1xK
	cv::Mat nk(cv::Mat::zeros(1, K, CV_32FC1));
	for (int n = 0; n < N; n++) {
		nk += gamma_32f.row(n);
	}

	// Ek0:1xK
	cv::Mat Ek0(nk.clone());
	Ek0 /= N;

	// Ek1:KxD
	cv::Mat Ek1(cv::Mat::zeros(K, D, CV_32FC1));
	for (int k = 0; k < K; k++) {
		tmp = cv::Mat::zeros(1, D, CV_32FC1);

		for (int n = 0; n < N; n++) {
			tmp += samples_32f.row(n)*gamma_32f.at<float>(n, k);
		}

		tmp /= nk.at<float>(k);
		tmp.copyTo(Ek1.row(k));
	}

	// Ek2:KxD
	cv::Mat Ek2(cv::Mat::zeros(K, D, CV_32FC1));
	for (int k = 0; k < K; k++) {
		tmp = cv::Mat::zeros(1, D, CV_32FC1);

		for (int n = 0; n < N; n++) {
			tmp += samples_32f.row(n).mul(samples_32f.row(n)) * gamma_32f.at<float>(n, k);
		}

		tmp /= nk.at<float>(k);
		tmp.copyTo(Ek2.row(k));
	}

	// r_vec:1xK
	cv::Mat r_vec(cv::Mat::ones(1, K, CV_32FC1)*r);

	// alpha:1xK   alpha2=1-alpha
	cv::Mat alpha(cv::Mat::zeros(1, K, CV_32FC1));
	alpha = nk / (nk + r_vec);
	cv::Mat alpha2(cv::Mat::ones(1, K, CV_32FC1));
	alpha2 -= alpha;

	cv::Mat orig_miu2(miu_32f.mul(miu_32f));


	pi_32f = pi_32f.mul(alpha2) + alpha.mul(Ek0);
	pi_32f /= cv::Mat::ones(1, K, CV_32FC1)*cv::sum(pi_32f)[0];

	for (int k = 0; k < K; k++) {
		miu_32f.row(k) =
			Ek1.row(k)*alpha.at<float>(k) +
			miu_32f.row(k)*alpha2.at<float>(k);
	}

	cv::Mat miu2(miu_32f.mul(miu_32f));
	for (int k = 0; k < K; k++) {
		sigma_32f.row(k) =
			Ek2.row(k)*alpha.at<float>(k) +
			(sigma_32f.row(k) + orig_miu2.row(k))*alpha2.at<float>(k) -
			miu2.row(k);
	}


	return std::make_tuple(miu_32f, pi_32f, sigma_32f);

}


// KL-normalization
// return value: miu,pi,sigma  (all converted to CV_32FC1)
std::tuple<cv::Mat, cv::Mat, cv::Mat> gmm_normalize(
	const cv::Mat &miu,
	const cv::Mat &pi, 
	const cv::Mat &sigma,
	const cv::Mat &adapted_miu,
	const cv::Mat &adapted_pi,
	const cv::Mat &adapted_sigma
	)
{
	// miu:KxD
	// pi:1xK
	// sigma:DxD x K (converted to KxD where each line consists of one diagonal)

	cv::Mat miu_32f = convertto32f(miu, false);                // the following codes will never modify xxx_32f.
	cv::Mat pi_32f = convertto32f(pi, false);
	cv::Mat sigma_32f = convertto32f(sigma, false);
	cv::Mat adapted_miu_32f = convertto32f(adapted_miu, false);
	cv::Mat adapted_pi_32f = convertto32f(adapted_pi, false);
	cv::Mat adapted_sigma_32f = convertto32f(adapted_sigma, false);

	int K = miu_32f.rows;
	int D = miu_32f.cols;

	cv::Mat miu_ret(cv::Mat::zeros(miu_32f.rows, miu_32f.cols, CV_32FC1));
	cv::Mat pi_ret(cv::Mat::zeros(pi_32f.rows, pi_32f.cols, CV_32FC1));
	cv::Mat sigma_ret(cv::Mat::zeros(sigma_32f.rows, sigma_32f.cols, CV_32FC1));

	cv::Mat tmp;

	for (int k = 0; k < K; k++) {
		cv::pow(sigma_32f.row(k), -0.5, tmp);
		miu_ret.row(k) = tmp.mul(adapted_miu_32f.row(k)) * (float)cv::sqrt(pi_32f.at<float>(k));

		cv::pow(sigma_32f.row(k), -1, tmp);
		sigma_ret.row(k) = tmp.mul(adapted_sigma_32f.row(k)) * (float)cv::sqrt(pi_32f.at<float>(k) / 2.0F);
	}

	adapted_pi_32f.copyTo(pi_ret);

	return std::make_tuple(miu_ret, pi_ret, sigma_ret);


}





#ifdef _DEBUG

#include "cvutils.h"
#include "utils.h"

void test_enc_gmm_em()
{
	const wchar_t *samplesfilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tif.txt";
	const wchar_t *miufilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tifmiu.txt";
	const wchar_t *pifilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tifpi.txt";
	const wchar_t *gammafilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tifgamma.txt";
	const wchar_t *sigmafilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tifsigma.txt";

	cv::Mat samples;

	csv2mat32f(samplesfilename, samples);

	cv::Mat samples_s(samples, cv::Rect(0, 0, 64, samples.rows));  // samples is REFERENCED by samples_s

	cv::Mat miu, pi, gamma, sigma;

	meassuretime();
	gmm_em(samples_s, 10, miu, pi, sigma, gamma);
	std::cout << "gmm_em: " << double(meassuretime()) / 1e9 << '\n';

	mat2csv(miufilename, miu);
	mat2csv(pifilename, pi);
	mat2csv(gammafilename, gamma);
	mat2csv(sigmafilename, sigma);
}


void test_enc_gmm_adapt()
{
	const wchar_t *samplesfilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tif.txt";
	const wchar_t *miufilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tifmiu.txt";
	const wchar_t *pifilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tifpi.txt";
	const wchar_t *sigmafilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tifsigma.txt";

	cv::Mat samples;

	csv2mat32f(samplesfilename, samples);

	cv::Mat samples_s(samples, cv::Rect(0, 0, 64, samples.rows));  // samples is REFERENCED by samples_s

	cv::Mat miu, pi, gamma, sigma;

	meassuretime();
	gmm_em(samples_s, 10, miu, pi, sigma, gamma);
	std::cout << "gmm_em: " << double(meassuretime()) / 1e9 << '\n';

	cv::Mat adapted_miu, adapted_pi, adapted_sigma;


	meassuretime();
	std::tie(adapted_miu, adapted_pi, adapted_sigma) = 
		gmm_adapt(samples_s, 32, miu, pi, sigma, gamma);
	std::cout << "gmm_adapt: " << double(meassuretime()) / 1e9 << '\n';

	mat2csv(miufilename, adapted_miu);
	mat2csv(pifilename, adapted_pi);
	mat2csv(sigmafilename, adapted_sigma);

}



void test_enc_gmm_normalize()
{
	const wchar_t *samplesfilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tif.txt";
	const wchar_t *miufilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tifmiu.txt";
	const wchar_t *pifilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tifpi.txt";
	const wchar_t *sigmafilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tifsigma.txt";

	cv::Mat samples;

	csv2mat32f(samplesfilename, samples);

	cv::Mat samples_s(samples, cv::Rect(0, 0, 64, samples.rows));  // samples is REFERENCED by samples_s



	cv::Mat miu, pi, gamma, sigma;

	meassuretime();
	gmm_em(samples_s, 10, miu, pi, sigma, gamma);
	std::cout << "gmm_em: " << double(meassuretime()) / 1e9 << '\n';



	cv::Mat adapted_miu, adapted_pi, adapted_sigma;

	meassuretime();
	std::tie(adapted_miu, adapted_pi, adapted_sigma) =
		gmm_adapt(samples_s, 32, miu, pi, sigma, gamma);
	std::cout << "gmm_adapt: " << double(meassuretime()) / 1e9 << '\n';



	cv::Mat norm_miu, norm_pi, norm_sigma;

	meassuretime();
	std::tie(norm_miu, norm_pi, norm_sigma) =
		gmm_normalize(miu, pi, sigma, adapted_miu, adapted_pi, adapted_sigma);
	std::cout << "gmm_normalize: " << double(meassuretime()) / 1e9 << '\n';



	mat2csv(miufilename, norm_miu);
	mat2csv(pifilename, norm_pi);
	mat2csv(sigmafilename, norm_sigma);

}



#endif