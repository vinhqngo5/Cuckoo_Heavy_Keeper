
#pragma once

unsigned int generate_uniform(unsigned int sizedom, double totalmass, vector<unsigned int> &f);
unsigned int generate_uniform_limited(unsigned int sizedom, double totalmass, double cutoff, vector<unsigned int> &f);
unsigned int generate_zipf(unsigned int sizedom, double totalmass, double zipf_param, vector<unsigned int> &f);
unsigned int generate_zipf_right(unsigned int sizedom, double totalmass, double zipf_param, double right_t, vector<unsigned int> &f);
unsigned int generate_reversed_zipf(unsigned int sizedom, double totalmass, double zipf_param, vector<unsigned int> &f);

void random_shuffling(vector<unsigned int> &f);
void random_shuffling(vector<unsigned int> &f, vector<unsigned int> &g);
void decorelate(vector<unsigned int> &f, double p_decor);
void print_freq_table(vector<unsigned int> &f);

class Relation {
  public:
    unsigned int dom_size;
    unsigned int tuples_no;
    vector<unsigned int> *tuples;

    Relation(unsigned int dom_size, unsigned int tuples_no);
    virtual ~Relation();

    void Generate_Data(int type, double data_param, double decor_param);
};

template <typename AppConfig> Relation *generate_relation(AppConfig &app_configs);

unsigned int generate_uniform(unsigned int sizedom, double totalmass, vector<unsigned int> &f) {
    unsigned int tuples_no = 0;
    totalmass = (sizedom > totalmass) ? sizedom : totalmass;

    for (unsigned int i = 0; i < sizedom; i++) {
        f[i] = (unsigned int) rint(totalmass / sizedom);
        tuples_no += f[i];
    }

    return tuples_no;
}

unsigned int generate_uniform_limited(unsigned int sizedom, double totalmass, double cutoff, vector<unsigned int> &f) {
    unsigned int tuples_no = 0;
    unsigned int new_domain = (unsigned int) rint(sizedom * cutoff);
    totalmass = (new_domain > totalmass) ? new_domain : totalmass;

    for (unsigned int i = 0; i < sizedom; i++) f[i] = 0;

    for (unsigned int i = 0; i < new_domain; i++) {
        f[i] = (unsigned int) rint(totalmass / new_domain);
        tuples_no += f[i];
    }

    return tuples_no;
}

unsigned int generate_zipf(unsigned int sizedom, double totalmass, double zipf_param, vector<unsigned int> &f) {
    if (zipf_param == 0.0) return generate_uniform(sizedom, totalmass, f);

    unsigned int tuples_no = 0;
    double normcoef = 0.0;
    for (unsigned int i = 0; i < sizedom; i++) normcoef += 1 / pow(i + 1, zipf_param);

    for (unsigned int i = 0; i < sizedom; i++) {
        f[i] = (unsigned int) rint(totalmass / (pow(i + 1, zipf_param) * normcoef));
        tuples_no += f[i];
    }

    return tuples_no;
}

unsigned int generate_zipf_right(unsigned int sizedom, double totalmass, double zipf_param, double right_t, vector<unsigned int> &f) {
    if (zipf_param == 0.0) return generate_uniform(sizedom, totalmass, f);

    unsigned int tuples_no = 0;
    double normcoef = 0.0;
    for (unsigned int i = 0; i < sizedom; i++) normcoef += 1 / pow(i + 1, zipf_param);

    for (unsigned int i = 0; i < sizedom; i++) {
        f[i] = (unsigned int) rint(totalmass / (pow(i + 1, zipf_param) * normcoef));
        tuples_no += f[i];
    }

    unsigned int right_m = (unsigned int) rint(right_t);
    unsigned int *t_f = new unsigned int[sizedom];
    for (unsigned int i = 0; i < sizedom; i++) t_f[i] = f[i];

    for (unsigned int i = 0; i < sizedom; i++) f[(i + right_m) % sizedom] = t_f[i];

    delete[] t_f;

    return tuples_no;
}

unsigned int generate_reversed_zipf(unsigned int sizedom, double totalmass, double zipf_param, vector<unsigned int> &f) {
    unsigned int tuples_no = 0;
    double normcoef = 0.0;
    for (unsigned int i = 0; i < sizedom; i++) normcoef += 1 / pow(i + 1, zipf_param);

    for (unsigned int i = 0; i < sizedom; i++) {
        f[i] = (unsigned int) rint(totalmass / (pow(sizedom - i, zipf_param) * normcoef));
        tuples_no += f[i];
    }

    return tuples_no;
}

void random_shuffling(vector<unsigned int> &f) {
    unsigned int sizedom = f.size();

    for (unsigned int i = 0; i < sizedom; i++) {
        unsigned int ip = rand() % sizedom;
        unsigned int tmp = f[i];
        f[i] = f[ip];
        f[ip] = tmp;
    }
}

void random_shuffling(vector<unsigned int> &f, vector<unsigned int> &g) {
    unsigned int sizedom = f.size();

    for (unsigned int i = 0; i < sizedom; i++) {
        unsigned int ip = rand() % sizedom;
        unsigned int tmp = f[i];
        f[i] = f[ip];
        f[ip] = tmp;

        tmp = g[i];
        g[i] = g[ip];
        g[ip] = tmp;
    }
}

void decorelate(std::vector<unsigned int> &f, double p_decor) {
    unsigned int sizedom = f.size();

    if (p_decor < 0) {
        for (int i = 0; i < (sizedom / 2); i++) {
            int temp = f[sizedom - 1 - i];
            f[sizedom - 1 - i] = f[i];
            f[i] = temp;
        }
    }

    for (int i = 0; i < sizedom * (1.0 - fabs(p_decor)); i++) {
        int ind1, ind2;
        ind1 = int(sizedom * drand48());
        ind2 = int(sizedom * drand48());

        int temp = f[ind2];
        f[ind2] = f[ind1];
        f[ind1] = temp;
    }
}

void print_freq_table(std::vector<unsigned int> &f) {
    for (unsigned int i = 0; i < f.size(); i++) std::cout << i << "\t" << f[i] << std::endl;
}

Relation::Relation(unsigned int dom_size, unsigned int tuples_no) {
    this->dom_size = dom_size;

    this->tuples_no = tuples_no;
    tuples = NULL;
}

Relation::~Relation() {
    dom_size = 0;
    tuples_no = 0;

    delete tuples;
    tuples = NULL;
}

void Relation::Generate_Data(int type, double data_param, double decor_param) {
    std::vector<unsigned int> freq(dom_size, 0);

    switch (type) {
    case 1: {
        tuples_no = generate_zipf(dom_size, tuples_no, data_param, freq);
        break;
    }
    case 2: {
        tuples_no = generate_reversed_zipf(dom_size, tuples_no, data_param, freq);
        break;
    }
    case 3: {
        tuples_no = generate_uniform(dom_size, tuples_no, freq);
        break;
    }
    case 4: {
        tuples_no = generate_uniform_limited(dom_size, tuples_no, data_param, freq);
        break;
    }
    case 5: {
        tuples_no = generate_zipf_right(dom_size, tuples_no, data_param, decor_param, freq);
        break;
    }
    }

    decorelate(freq, decor_param);

    tuples = new std::vector<unsigned int>(tuples_no, 0);
    // temporarily not calculate 0 frequency
    tuples_no -= freq[0];

    int c_tup = 0;
    for (int i = 1; i <= dom_size; i++) {
        for (int j = 0; j < freq[i]; j++) { (*tuples)[c_tup + j] = i; }
        c_tup += freq[i];
    }
}

template <typename AppConfig> Relation *generate_relation(AppConfig &app_configs) {
    Relation *r1 = new Relation(app_configs.DOM_SIZE, app_configs.tuples_no);
    r1->Generate_Data(app_configs.DIST_TYPE, app_configs.DIST_PARAM, app_configs.DIST_SHUFF);

    // Sometimes Generate_Data might generate less than tuples_no
    if (r1->tuples_no < app_configs.tuples_no) { app_configs.tuples_no = r1->tuples_no; }

    // shuffle the tuples
    auto rng = default_random_engine{};
    shuffle(begin((*r1->tuples)), begin((*r1->tuples)) + app_configs.tuples_no, rng);
    return r1;
}
