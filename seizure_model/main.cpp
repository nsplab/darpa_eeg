#include <iostream>
#include <xdfio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fstream>

#include <eigen3/Eigen/Dense>

using namespace Eigen;
using namespace std;

void WriteCSV(MatrixXd &mat, ofstream &file) {
    file.precision(12);
    for (size_t i=0; i<mat.rows(); i++) {
        size_t j=0;
        for (; j<mat.cols()-1; j++) {
            file<<mat(i,j)<<",";
        }
        file<<mat(i,j)<<endl;
    }
}

int main()
{
  /*
    File Name: chb01_03.edf
    File Start Time: 13:43:04
    File End Time: 14:43:04
    Number of Seizures in File: 1
    Seizure Start Time: 2996 seconds
    Seizure End Time: 3036 seconds
    Sampling rate: 256 Hz
    Channel 1: FP1-F7
    Channel 2: F7-T7
    Channel 3: T7-P7
    Channel 4: P7-O1
    Channel 5: FP1-F3
    Channel 6: F3-C3
    Channel 7: C3-P3
    Channel 8: P3-O1
    Channel 9: FP2-F4
    Channel 10: F4-C4
    Channel 11: C4-P4
    Channel 12: P4-O2
    Channel 13: FP2-F8
    Channel 14: F8-T8
    Channel 15: T8-P8
    Channel 16: P8-O2
    Channel 17: FZ-CZ
    Channel 18: CZ-PZ
    Channel 19: P7-T7
    Channel 20: T7-FT9
    Channel 21: FT9-FT10
    Channel 22: FT10-T8
    Channel 23: T8-P8
  */
  const size_t seizureStartTime = 2996;
  const size_t seizureEndTime = 3036;
  const size_t samplingRate = 256;
  const size_t seizureSampleStart = seizureStartTime * samplingRate;
  const size_t seizureSampleEnd = seizureEndTime * samplingRate;

  ////////////////////////////////////////////////////////////////////////////
  // Get header data
  ////////////////////////////////////////////////////////////////////////////
  xdf* xfile = xdf_open("../data/chb01_03.edf", XDF_READ, XDF_EDF);
  int numChannels = 0;
  int frq = 0;
  xdf_get_conf(xfile, XDF_F_NCHANNEL, &numChannels, XDF_F_SAMPLING_FREQ, &frq, XDF_NOF);
  cout<<"sampling frquency: "<<frq<<endl;
  cout<<"number of channels: "<<numChannels<<endl;

  size_t samplesize = numChannels*sizeof(int16_t);
  int16_t* buffer = (int16_t *)malloc(samplesize);

  size_t stride[1];
  stride[0] = samplesize;

  xdf_define_arrays(xfile, 1, stride);
  xdf_prepare_transfer(xfile);

  xdftype type;
  double physicalMax = 0;
  double physicalMin = 0;
  double digitalMin = 0;
  double digitalMax = 0;
  for (size_t ch=0; ch<1; ch++) {
      xdfch* xch = xdf_get_channel(xfile, 0);
      xdf_get_chconf(xch, XDF_CF_STOTYPE, &type, XDF_CF_PMIN, &physicalMin,
                     XDF_CF_PMAX, &physicalMax, XDF_CF_DMIN, &digitalMin,
                     XDF_CF_DMAX, &digitalMax, XDF_NOF);
      cout<<"channel "<<ch<<" type: "<<type<<endl;
  }

  double scaleFac = (physicalMax - physicalMin) / (digitalMax - digitalMin);
  double dc = physicalMax - scaleFac * digitalMax;

  ////////////////////////////////////////////////////////////////////////////
  /// Compute mean and variance of non-seizure signals
  ////////////////////////////////////////////////////////////////////////////
  ofstream nonseizMeanFile("nonseizure_mean.csv");
  ofstream nonseizMovFile("nonseizure_covariance.csv");

  MatrixXd nonseizureSamples(seizureSampleStart, numChannels);

  for (size_t s=0; s<seizureSampleStart; s++) {
      size_t nssrc = xdf_read(xfile, 1, buffer);

      double sum = 0;
      for (int i=(numChannels-1); i>=0; i--) {
          nonseizureSamples(s, i) = buffer[i] * scaleFac + dc; + sum;
          sum += nonseizureSamples(s, i);
      }
  }

  VectorXd oneVec;
  oneVec.setOnes(seizureSampleStart);

  MatrixXd mean = nonseizureSamples.colwise().mean();
  MatrixXd centered = nonseizureSamples - oneVec * mean;
  MatrixXd cov = (centered.transpose() * centered)/(seizureSampleStart - 1.0);

  WriteCSV(mean, nonseizMeanFile);
  WriteCSV(cov, nonseizMovFile);

  ////////////////////////////////////////////////////////////////////////////
  /// Compute mean and variance of seizure signals
  ////////////////////////////////////////////////////////////////////////////
  ofstream seizMeanFile("seizure_mean.csv");
  ofstream seizMovFile("seizure_covariance.csv");

  size_t numSeizureSamples = seizureSampleEnd - seizureSampleStart;
  MatrixXd seizureSamples(numSeizureSamples, numChannels);

  xdf_seek(xfile, seizureSampleStart, SEEK_SET);
  for (size_t s=0; s<numSeizureSamples; s++) {
      size_t nssrc = xdf_read(xfile, 1, buffer);

      double sum = 0;
      for (int i=(numChannels-1); i>=0; i--) {
          seizureSamples(s, i) = buffer[i] * scaleFac + dc + sum;
          sum += seizureSamples(s, i);
      }
  }

  oneVec.setOnes(numSeizureSamples);

  mean = seizureSamples.colwise().mean();
  centered = seizureSamples - oneVec * mean;
  cov = (centered.transpose() * centered)/(numSeizureSamples - 1.0);

  WriteCSV(mean, seizMeanFile);
  WriteCSV(cov, seizMovFile);

  // close edf file
  xdf_close(xfile);

  return 0;
}

