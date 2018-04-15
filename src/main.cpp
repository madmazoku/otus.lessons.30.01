
#include "../bin/version.h"

#include <iostream>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <spdlog/spdlog.h>

#include <SDL2/SDL.h>
// #include <SDL2/SDL_image.h>

#include <dlib/clustering.h>

#include <cmath>

void hsv2rgb(Uint8 h, Uint8 s, Uint8 v, Uint8& r, Uint8& g, Uint8& b)
{
    unsigned char region, remainder, p, q, t;

    if (s == 0)
    {
        r = v;
        g = v;
        b = v;
        return;
    }

    region = h / 43;
    remainder = (h - (region * 43)) * 6; 

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            r = v; g = t; b = p;
            break;
        case 1:
            r = q; g = v; b = p;
            break;
        case 2:
            r = p; g = v; b = t;
            break;
        case 3:
            r = p; g = q; b = v;
            break;
        case 4:
            r = t; g = p; b = v;
            break;
        default:
            r = v; g = p; b = q;
            break;
    }
}

void main_body();

int main(int argc, char** argv)
{
    auto console = spdlog::stderr_logger_st("console");
    console->info("Wellcome!");

    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "print usage message")
    ("version,v", "print version number");

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
    } else if (vm.count("version")) {
        std::cout << "Build version: " << build_version() << std::endl;
        std::cout << "Boost version: " << (BOOST_VERSION / 100000) << '.' << (BOOST_VERSION / 100 % 1000) << '.' << (BOOST_VERSION % 100) << std::endl;
    } else {
        main_body();
    }

    console->info("Goodby!");

    return 0;
}

void main_body()
{
    auto console = spdlog::get("console");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        console->error("SDL_Init Error: {0}", SDL_GetError());
        throw std::runtime_error("SDL_Init");
    }

    SDL_DisplayMode display_mode;
    if (SDL_GetCurrentDisplayMode(0, &display_mode) != 0) {
        console->error("SDL_GetCurrentDisplayMode Error: {0}", SDL_GetError());
        SDL_Quit();
        throw std::runtime_error("SDL_GetCurrentDisplayMode");
    }

    long width = long(display_mode.w * 0.75);
    long height = long(display_mode.h * 0.75);

    if(width < height)
        height = width;
    else
        width = height;

    SDL_Window *win = SDL_CreateWindow(
                          "Hellow World!",
                          SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED,
                          width,
                          height,
                          SDL_WINDOW_SHOWN
                      );
    if (win == nullptr) {
        console->error("SDL_CreateWindow Error: {0}", SDL_GetError());
        SDL_Quit();
        throw std::runtime_error("SDL_CreateWindow");
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (win == nullptr) {
        console->error("SDL_CreateRenderer Error: {0}", SDL_GetError());
        SDL_Quit();
        throw std::runtime_error("SDL_CreateRenderer");
    }

    // SDL_Surface *scr = SDL_GetWindowSurface(win);
    // SDL_Surface *img = SDL_Createurface(0, scr->w, scr->h, 32, 0, 0, 0, 0);

    typedef dlib::matrix<double,2,1> sample_type;
    typedef dlib::radial_basis_kernel<sample_type> kernel_type;
    dlib::kcentroid<kernel_type> kc(kernel_type(0.1),0.01, 8);
    dlib::kkmeans<kernel_type> test(kc);

    std::vector<sample_type> samples;
    std::vector<sample_type> initial_centers;

    std::vector<long unsigned int> clustering;
    std::string line;
    std::map<int, std::vector<std::tuple<double, double, int>>> clusters;
    while(std::getline(std::cin, line)) {
        std::vector<std::string> tokens;

        boost::char_separator<char> sep{";\n", " "};
        boost::tokenizer<boost::char_separator<char>> tok{line, sep};
        std::copy( tok.begin(), tok.end(), std::back_inserter(tokens) );

        double x = std::stof(tokens[0]);
        double y = std::stof(tokens[1]);
        int c = tokens.size() < 3 ? 0 : std::stoi(tokens[2]);

        clusters[c].push_back(std::make_tuple(x, y, c));

        sample_type m;
        m(0) = x;
        m(1) = y;

        samples.push_back(m);
        clustering.push_back(c);
    }
    console->info("Loaded {0} samples and {1} clusters", samples.size(), clusters.size());

    test.set_number_of_centers(clusters.size());
    dlib::pick_initial_centers(test.number_of_centers(), initial_centers, samples, test.get_kernel());
    test.train(samples,initial_centers);

    std::vector<long unsigned int> assignments = dlib::spectral_cluster(kernel_type(0.1), samples, clusters.size());

    // dlib::find_clusters_using_kmeans(samples, initial_centers);
    // std::vector<unsigned long> assignments;
    // for (unsigned long i = 0; i < samples.size(); ++i)
    //     assignments.push_back(nearest_center(initial_centers, samples[i]));

    for(size_t n = 0; n < samples.size(); ++n) {
        sample_type m = samples[n];
        std::cout << m(0) << ";" << m(1) << ";" << assignments[n] << std::endl;
    }

    std::vector<std::tuple<int, size_t, SDL_Point*>> samples_p;
    for(auto& ss : clusters) {
        SDL_Point* points = new SDL_Point[ss.second.size()];
        for(size_t n = 0; n < ss.second.size(); ++n) {
            points[n].x = int(double(width - 10) * (std::get<0>(ss.second[n]) + 100) / 200) + 5;
            points[n].y = int(double(height - 10) * (std::get<1>(ss.second[n]) + 100) / 200) + 5;
        }
        samples_p.push_back(std::make_tuple(ss.first, ss.second.size(), points));
    }

    for(size_t n = 0; n < samples_p.size(); ++n)
        console->info("Cluster # {0}; id: {1}; points: {2}", n, std::get<0>(samples_p[n]), std::get<1>(samples_p[n]));

    bool run = true;

    auto start = std::chrono::system_clock::now();
    size_t count = 0;
    auto last = start;
    size_t last_count = count;
    double time_step = 0.0;
    bool orig = true;

    while (run) {
        auto loop_start = std::chrono::system_clock::now();
        ++count;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Rect rect;
        rect.x = 5;
        rect.y = 5;
        rect.w = width - 10;
        rect.h = height - 10;
        SDL_SetRenderDrawColor(renderer, 0x7f, 0x7f, 0x7f, 255);
        SDL_RenderDrawRect(renderer, &rect);

        // size_t N = 20;
        // double sx = double(width - 10) / N;
        // double sy = double(height - 10) / N;

        // for(size_t n = 0; n < 5; ++n) {
        //     SDL_SetRenderDrawColor(renderer, 0x7f + n * 0x10, 0x7f + n * 0x10, 0x7f + n * 0x10, 255);
        //     size_t m = size_t(n + count * 10 * time_step) % N;
        //     SDL_RenderDrawLine(renderer, 5, 5 + m * sy, 5 + m * sx, height - 5);
        //     SDL_RenderDrawLine(renderer, 5 + (N - m) * sx, 5, width - 5, 5 + (N - m) * sy);

        //     SDL_RenderDrawLine(renderer, 5, 5 + m * sy, 5 + (N - m) * sx, 5);
        //     SDL_RenderDrawLine(renderer, 5 + m * sx, height - 5, width - 5, 5 + (N - m) * sy);
        // }

        // size_t x = (sin(M_PI * count * time_step) + 2.0) * (width >> 2);
        // size_t y = (cos(M_PI * count * time_step) + 2.0) * (height >> 2);
        // SDL_RenderDrawLine(renderer, 5, y, width - 5, y);
        // SDL_RenderDrawLine(renderer, x, 5, x, height - 5);

        double sha = 0x7f * (1.0 / test.number_of_centers());
        double shc = 0x7f * (1.0 / clusters.size());
        for(size_t n = 0; n < samples.size(); ++n) {
            sample_type m = samples[n];
            int a;

            Uint8 h,s,v;
            h = 0;
            s = 0xff;
            v = 0xff;

            if(orig)
                h = 0xff - clustering[n] * shc;
            else
                h = assignments[n] * sha;

            Uint8 r,g,b;
            hsv2rgb(h, s, v, r, g, b);

            SDL_Rect rectangle;
            rectangle.w = 3;
            rectangle.h = 3;

            int x = (width - 10) * (m(0) + 100) / 200 + 5;
            int y = (height - 10) * (m(1) + 100) / 200 + 5;

            rectangle.x = x - 1;
            rectangle.y = y - 1;

            SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
            SDL_RenderFillRect(renderer, &rectangle);

        }

        // double sh = 255.0 / samples_p.size();
        // for(size_t n = 0; n < samples_p.size(); ++n)
        // {
        //     Uint8 h,s,v;
        //     h = Uint8(n * sh);
        //     s = 0xff;
        //     v = 0xff;

        //     Uint8 r,g,b;
        //     hsv2rgb(h, s, v, r, g, b);

        //     SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
        //     SDL_RenderDrawPoints(renderer, std::get<2>(samples_p[n]), std::get<1>(samples_p[n]));
        // }

        // SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 255);
        // SDL_RenderDrawPoints(renderer, samples_p, samples.size());

        SDL_RenderPresent(renderer);

        // SDL_LockSurface(img);
        // SDL_memset((unsigned char*)(img->pixels), 0xff, img->h * img->pitch);
        // for(size_t y = 0; y < img->h; ++y)
        //     for(size_t x = 0; x < img->w; ++x) {
        //         unsigned char* pixel = (unsigned char*)(img->pixels) + x * 4 + y * img->pitch;
        //         Uint8 lumr = x + count;
        //         Uint8 lumg = y + count;
        //         Uint8 lumb = x + y + count;
        //         *((Uint32*)pixel) = SDL_Map(img->format, lumr, lumg, lumb, 0x00);
        //     }
        // SDL_UnlockSurface(img);
        // SDL_BlitSurface(img, nullptr, scr, nullptr);

        // SDL_UpdateWindowSurface(win);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT
                || event.type == SDL_KEYDOWN
                || event.type == SDL_KEYUP) {
                run = false;
            }
        }

        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> full_elapsed = end - start;
        std::chrono::duration<double> last_elapsed = end - last;
        std::chrono::duration<double> loop_elapsed = end - loop_start;
        time_step = (time_step * (count-1) + loop_elapsed.count()) / count;

        if (!run || last_elapsed.count() >= 1) {
            int frames = count - last_count;
            double fps = ((double)frames) / last_elapsed.count();

            orig = long(count * time_step * 0.5) % 2;
            std::string title = "kkmeans: ";
            title += orig ? "original" : "calculated";
            title += " FPS: " + std::to_string(fps);

            SDL_SetWindowTitle(win, title.c_str());

            console->info("[{0} / {1}] fps: {2}; time_step: {3}", full_elapsed.count(), count, fps, time_step);

            last = end;
            last_count = count;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
}