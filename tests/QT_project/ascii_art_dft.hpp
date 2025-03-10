#ifndef ASCII_ART_DFT_HPP
#define ASCII_ART_DFT_HPP

#include <QString>
#include <QVector>
#include <QDebug>
#include <QTextStream>
#include <cmath>
#include <complex>
#include <stdexcept>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

namespace ascii_art_dft {

    typedef QVector<float> log_pwr_dft_type;

    /*!
     * Get a logarithmic power DFT of the input samples.
     * Samples are expected to be in the range [-1.0, 1.0].
     * \param samps a pointer to an array of complex samples
     * \param nsamps the number of samples in the array
     * \return a real range of DFT bins in units of dB
     */
    template <typename T> log_pwr_dft_type log_pwr_dft(
        const std::complex<T> *samps, size_t nsamps
    );

    /*!
     * Convert a DFT to a printable ascii plot.
     * \param dft the log power dft bins
     * \param width the frame width in characters
     * \param height the frame height in characters
     * \param samp_rate the sample rate in Sps
     * \param dc_freq the DC frequency in Hz
     * \param dyn_rng the dynamic range in dB
     * \param ref_lvl the reference level in dB
     * \return the plot as an ascii string
     */
    QString dft_to_plot(
        const log_pwr_dft_type &dft,
        size_t width,
        size_t height,
        double samp_rate,
        double dc_freq,
        float dyn_rng,
        float ref_lvl
    );

} // namespace ascii_art_dft

/***********************************************************************
 * Implementation includes
 **********************************************************************/
#include <cmath>
#include <QVector>
#include <QString>
#include <QDebug>
#include <QTextStream>

/***********************************************************************
 * Helper functions
 **********************************************************************/
namespace {/*anon*/

    static const double pi = double(std::acos(-1.0));

    //! Round a floating-point value to the nearest integer
    template <typename T> int iround(T val){
        return (val > 0) ? int(val + 0.5) : int(val - 0.5);
    }

    //! Pick the closest number that is nice to display
    template <typename T> T to_clean_num(const T num){
        if (num == 0) return 0;
        const T pow10 = std::pow(T(10), int(std::floor(std::log10(std::abs(num)))));
        const T norm = std::abs(num)/pow10;
        static const int cleans[] = {1, 2, 5, 10};
        int clean = cleans[0];
        for (size_t i = 1; i < sizeof(cleans)/sizeof(cleans[0]); i++){
            if (std::abs(norm - cleans[i]) < std::abs(norm - clean))
                clean = cleans[i];
        }
        return ((num < 0) ? -1 : 1) * clean * pow10;
    }

    //! Compute an FFT with pre-computed factors using Cooley-Tukey
    template <typename T> std::complex<T> ct_fft_f(
        const std::complex<T> *samps, size_t nsamps,
        const std::complex<T> *factors,
        size_t start = 0, size_t step = 1
    ){
        if (nsamps == 1) return samps[start];
        std::complex<T> E_k = ct_fft_f(samps, nsamps/2, factors+1, start,      step*2);
        std::complex<T> O_k = ct_fft_f(samps, nsamps/2, factors+1, start+step, step*2);
        return E_k + factors[0]*O_k;
    }

    //! Compute an FFT for a particular bin k using Cooley-Tukey
    template <typename T> std::complex<T> ct_fft_k(
        const std::complex<T> *samps, size_t nsamps, size_t k
    ){
        // Pre-compute the factors to use in Cooley-Tukey
        QVector<std::complex<T>> factors;
        for (size_t N = nsamps; N != 0; N /= 2){
            factors.push_back(std::exp(std::complex<T>(0, T(-2 * pi * k / N))));
        }
        return ct_fft_f(samps, nsamps, &factors.front());
    }

    //! Helper class to build a DFT plot frame
    class frame_type{
    public:
        frame_type(size_t width, size_t height)
            : _frame(width - 1, QVector<char>(height, ' '))
        {
            /* NOP */
        }

        // Accessors to parts of the frame
        char &get_plot(size_t b, size_t z){return _frame[b + albl_w][z + flbl_h];}
        char &get_albl(size_t b, size_t z){return _frame[b][z + flbl_h];}
        char &get_ulbl(size_t b)          {return _frame[b][flbl_h - 1];}
        char &get_flbl(size_t b)          {return _frame[b + albl_w][flbl_h - 1];}

        // Dimension accessors
        size_t get_plot_h(void) const{return _frame.first().size() - flbl_h;}
        size_t get_plot_w(void) const{return _frame.size() - albl_w;}
        size_t get_albl_w(void) const{return albl_w;}

        QString to_string(void){
            QString frame_str;
            for (size_t z = 0; z < _frame.first().size(); z++){
                for (size_t b = 0; b < _frame.size(); b++){
                    frame_str += _frame[b][_frame[b].size() - z - 1];
                }
                frame_str += '\n';
            }
            return frame_str;
        }

    private:
        static const size_t albl_w = 6, flbl_h = 1;
        QVector<QVector<char>> _frame;
    };

} // namespace /*anon*/

/***********************************************************************
 * Implementation code
 **********************************************************************/
namespace ascii_art_dft{

    static const size_t albl_skip = 5, flbl_skip = 20;

    template <typename T> log_pwr_dft_type log_pwr_dft(
        const std::complex<T> *samps, size_t nsamps
    ){
        if (nsamps & (nsamps - 1))
            throw std::runtime_error("num samps is not a power of 2");

        // Compute the window
        double win_pwr = 0;
        QVector<std::complex<T>> win_samps;
        for(size_t n = 0; n < nsamps; n++){
            double w_n = 0.35875 // Blackman-Harris window
                -0.48829*std::cos(2 * pi * n / (nsamps - 1))
                +0.14128*std::cos(4 * pi * n / (nsamps - 1))
                -0.01168*std::cos(6 * pi * n / (nsamps - 1))
            ;
            win_samps.push_back(T(w_n) * samps[n]);
            win_pwr += w_n * w_n;
        }

        // Compute the log-power DFT
        log_pwr_dft_type log_pwr_dft(nsamps);
        for(size_t k = 0; k < nsamps; k++){
            std::complex<T> dft_k = ct_fft_k(&win_samps.front(), nsamps, k);
            log_pwr_dft[k] = float(
                + 20 * std::log10(std::abs(dft_k))
                - 20 * std::log10(T(nsamps))
                - 10 * std::log10(win_pwr / nsamps)
                + 3
            );
        }

        return log_pwr_dft;
    }

    QString dft_to_plot(
        const log_pwr_dft_type &dft_,
        size_t width,
        size_t height,
        double samp_rate,
        double dc_freq,
        float dyn_rng,
        float ref_lvl
    ){
        frame_type frame(width, height); // fill this frame

        // Re-order the DFT so DC is in the center
        const size_t num_bins = dft_.size() - 1 + dft_.size() % 2; // make it odd
        log_pwr_dft_type dft(num_bins);
        for (size_t n = 0; n < num_bins; n++){
            dft[n] = dft_[(n + num_bins / 2) % num_bins];
        }

        // Fill the plot with DFT bins
        for (size_t b = 0; b < frame.get_plot_w(); b++) {
            const size_t n_start = std::max(iround(double(b - 0.5) * (num_bins - 1) / (frame.get_plot_w() - 1)), 0);
            const size_t n_stop = std::min(static_cast<size_t>(iround(double(b + 0.5) * (num_bins - 1) / (frame.get_plot_w() - 1))), num_bins);

            float val = dft.at(n_start);
            for (size_t n = n_start; n < n_stop; n++) val = std::max(val, dft.at(n));

            const float scaled = (val - (ref_lvl - dyn_rng)) * (frame.get_plot_h() - 1) / dyn_rng;
            for (size_t z = 0; z < frame.get_plot_h(); z++) {
                static const std::string syms(".:!|");
                if (scaled - z >= 1) frame.get_plot(b, z) = syms.at(syms.size() - 1);
                else if (scaled - z > 0) frame.get_plot(b, z) = syms.at(size_t((scaled - z) * syms.size()));
    }
}


        // Create vertical amplitude labels
        const float db_step = to_clean_num(dyn_rng / (frame.get_plot_h() - 1) * albl_skip);
        for (
            float db = db_step * int((ref_lvl - dyn_rng) / db_step);
            db <= db_step * int(ref_lvl / db_step);
            db += db_step
        ){
            const int z = iround((db - (ref_lvl - dyn_rng)) * (frame.get_plot_h() - 1) / dyn_rng);
            if (z < 0 || size_t(z) >= frame.get_plot_h()) continue;
            QString lbl = QString::number(db);
            for (size_t i = 0; i < lbl.size() && i < frame.get_albl_w(); i++){
                frame.get_albl(i, z) = lbl.toLatin1().at(i);
            }
        }

        // Create vertical units label
        QString ulbl = "dBfs";
        for (size_t i = 0; i < ulbl.size(); i++){
            frame.get_ulbl(i + 1) = ulbl.toLatin1().at(i);

        }

        // Create horizontal frequency labels
        const double f_step = to_clean_num(samp_rate / frame.get_plot_w() * flbl_skip);
        for (
            double freq = f_step * int((-samp_rate / 2 / f_step));
            freq <= f_step * int((+samp_rate / 2 / f_step));
            freq += f_step
        ){
            const int b = iround((freq + samp_rate / 2) * (frame.get_plot_w() - 1) / samp_rate);
            QString lbl = QString::number((freq + dc_freq) / 1e6, 'f', 2) + "MHz";
            if (b < int(lbl.size() / 2) || b + lbl.size() - lbl.size() / 2 >= frame.get_plot_w()) continue;
            for (size_t i = 0; i < lbl.size(); i++){
                frame.get_flbl(b + i - lbl.size() / 2) = lbl.toLatin1().at(i);
            }
        }

        return frame.to_string();
    }
} // namespace ascii_art_dft

#endif /* ASCII_ART_DFT_HPP */

