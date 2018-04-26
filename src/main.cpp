
#include "../bin/version.h"

#include <iostream>
#include <cmath>

#include <boost/tokenizer.hpp>

#include <dlib/clustering.h>
#include <dlib/image_io.h>
#include <dlib/image_transforms.h>

typedef dlib::matrix<double,2,1> sample_type;
typedef dlib::linear_kernel<sample_type> kernel_type;

double sqr(double x)
{
    return x * x;
}

struct Cluster {
    size_t _idx;
    sample_type _center;
    std::vector<sample_type> _samples;

    double _mean;
    double _dev;

    Cluster(size_t idx, sample_type center) noexcept : _idx(idx), _center(center), _mean(0), _dev(0) {}
    Cluster(const Cluster&) = default;
    Cluster(Cluster&&) = default;

    Cluster& operator=(const Cluster&) = default;
    Cluster& operator=(Cluster&&) = default;
};

int main(int argc, char** argv)
{
    if(argc < 2) {
        std::cout << "Usage: " << argv[0] << " N [img_size]" << std::endl;
        std::cout << "\tdefault img_size is 200" << std::endl;
        return 0;
    }

    size_t N = std::stol(argv[1]);

    size_t img_size = argc > 2 ? std::atol(argv[2]) : 200;

    dlib::kcentroid<kernel_type> kc(kernel_type(),0.01, 8);
    dlib::kkmeans<kernel_type> test(kc);

    std::vector<sample_type> samples;
    std::vector<sample_type> centers;

    std::string line;
    size_t lines = 0;
    while(std::getline(std::cin, line)) {
        ++lines;

        std::vector<std::string> tokens;

        boost::char_separator<char> sep{";\n", " "};
        boost::tokenizer<boost::char_separator<char>> tok{line, sep};
        std::copy( tok.begin(), tok.end(), std::back_inserter(tokens) );

        double x = std::stof(tokens[0]);
        double y = std::stof(tokens[1]);

        if(x < -100 || x > 100 || y < -100 || y > 100) {
            std::cerr << "skip sample out of bounders in " << lines << " line" << std::endl;
        }

        sample_type m;
        m(0) = x;
        m(1) = y;

        samples.push_back(m);
    }
    std::cerr << "Loaded " << samples.size() << " samples" << std::endl;
    std::cerr << "Divide into " << N << " clusters by kmeans" << std::endl;

    test.set_number_of_centers(N);
    dlib::pick_initial_centers(test.number_of_centers(), centers, samples, test.get_kernel());
    dlib::find_clusters_using_kmeans(samples, centers);
    test.train(samples,centers);

    std::vector<Cluster> clusters;
    for(size_t n = 0; n < test.number_of_centers(); ++n)
        clusters.push_back(Cluster(n, centers[n]));

    std::ofstream kmeans_out("kmeans.csv");
    for (size_t n = 0; n < samples.size(); ++n) {
        sample_type& s = samples[n];
        size_t idx = test(s);
        kmeans_out << s(0) << ";" << s(1) << ";" << idx << "\n";
        clusters[idx]._samples.push_back(s);
    }
    kmeans_out.close();

    std::ofstream centers_out("centers.csv");
    size_t max_size = 0;
    for(auto& c : clusters) {
        centers_out << c._idx << ";" << c._center(0) << ";" << c._center(1) << ";" << c._samples.size() << "\n";
        if(max_size < c._samples.size())
            max_size = c._samples.size();

        double d = 0;
        for(auto& s : c._samples)
            d += sqrt(sqr(c._center(0) - s(0)) + sqr(c._center(1) - s(1)));
        c._mean = d / c._samples.size();
        d = 0;
        for(auto& s : c._samples)
            d += sqr(c._mean - sqrt(sqr(c._center(0) - s(0)) + sqr(c._center(1) - s(1))));
        c._dev = sqrt(d / (c._samples.size() - 1));
        std::cerr << "Cluster # " << c._idx
                  << " { " << c._center(0) << ", " << c._center(1) << " }"
                  << " has " << c._samples.size() << " samples;"
                  << " mean: " << c._mean
                  << " deviation: " << c._dev
                  << std::endl;
    }
    centers_out.close();

    dlib::array2d<dlib::hsi_pixel> img(img_size, img_size);
    double scale_size = 0.5 * 100 / N / max_size;
    double scale_space = img_size / 200.0;
    double scale_color = 255.0 / N;
    double scale_dist = 255.0 / sqrt(2 * sqr(img_size));

    for(size_t y = 0; y < img_size; ++y)
        for(size_t x = 0; x < img_size; ++x) {
            sample_type m;
            m(0) = x / scale_space - 100.0;
            m(1) = y / scale_space - 100.0;
            Cluster& c = clusters[test(m)];
            double d = sqrt(sqr(c._center(0) - m(0)) + sqr(c._center(1) - m(1)));
            bool b_mean = fabs(d - c._mean) * scale_space < 1.0;
            bool b_size = fabs(d) < c._samples.size() * scale_size;
            bool b_dev = fabs(d - c._mean) < c._dev;
            if( b_mean ) {
                uint8_t h = uint8_t(c._idx * scale_color);
                img[y][x] = dlib::hsi_pixel(h, 0x7f, 0x3f);
            } else if( b_size ) {
                uint8_t h = uint8_t(c._idx * scale_color);
                img[y][x] = dlib::hsi_pixel(h, 0x7f, 0x7f);
            } else if( b_dev ) {
                uint8_t h = uint8_t(c._idx * scale_color);
                img[y][x] = dlib::hsi_pixel(h, 0x7f, 0x1f);
            }
        }

    for(auto& c : clusters) {
        uint8_t h = uint8_t(c._idx * scale_color);
        size_t cx = size_t((c._center(0) + 100) * scale_space);
        size_t cy = size_t((c._center(1) + 100) * scale_space);
        size_t cm = size_t(c._mean * scale_space);
        size_t cr = c._samples.size() * scale_size;
        // dlib::draw_solid_circle(img, dlib::dpoint(cx, cy), cr, dlib::hsi_pixel(h, 0x7f, 0x3f));
        for(auto& s : c._samples) {
            size_t x = size_t((s(0) + 100) * scale_space);
            size_t y = size_t((s(1) + 100) * scale_space);
            img[y][x] = dlib::hsi_pixel(h, 0xff, 0x7f);
        }
    }

    dlib::save_png(img, "./kmeans.png");

    return 0;
}