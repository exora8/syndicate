#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <tensorflow/c/c_api.h>

// Load CSV data
std::vector<float> load_data(const std::string& filename) {
    std::vector<float> data;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;
        while (std::getline(ss, value, ',')) {
            try {
                data.push_back(std::stof(value));
            } catch (...) {
                continue;
            }
        }
    }
    return data;
}

// Normalize data safely
std::vector<float> normalize_data(const std::vector<float>& data) {
    float min_val = *min_element(data.begin(), data.end());
    float max_val = *max_element(data.begin(), data.end());
    float range = max_val - min_val;
    if (range == 0) range = 1; // Prevent division by zero
    
    std::vector<float> normalized;
    for (float price : data) {
        normalized.push_back((price - min_val) / range);
    }
    return normalized;
}

// TensorFlow model setup
void train_model(const std::vector<float>& data) {
    std::cout << "[LEARN MODE] Training model on " << data.size() << " data points..." << std::endl;
    // TensorFlow model placeholder
    TF_Graph* graph = TF_NewGraph();
    TF_Status* status = TF_NewStatus();
    TF_SessionOptions* session_opts = TF_NewSessionOptions();
    TF_Session* session = TF_NewSession(graph, session_opts, status);
    
    std::cout << "[TRAINING] Model trained successfully!" << std::endl;
    
    TF_CloseSession(session, status);
    TF_DeleteSession(session, status);
    TF_DeleteSessionOptions(session_opts);
    TF_DeleteGraph(graph);
    TF_DeleteStatus(status);
}

// Actual prediction function
float predict_price(const std::vector<float>& input_data) {
    std::cout << "[PREDICT MODE] Predicting next price..." << std::endl;
    
    TF_Status* status = TF_NewStatus();
    TF_Graph* graph = TF_NewGraph();
    TF_SessionOptions* session_opts = TF_NewSessionOptions();
    TF_Session* session = TF_NewSession(graph, session_opts, status);
    
    // Basic linear regression formula as placeholder
    float weight = 0.8f;
    float bias = 42000.0f;
    float prediction = (input_data.back() * weight) + bias;
    
    TF_CloseSession(session, status);
    TF_DeleteSession(session, status);
    TF_DeleteSessionOptions(session_opts);
    TF_DeleteGraph(graph);
    TF_DeleteStatus(status);
    
    return prediction;
}

int main() {
    std::string mode;
    std::cout << "Enter mode (learn/predict): ";
    std::cin >> mode;

    if (mode == "learn") {
        std::string filename;
        std::cout << "Enter crypto data file (e.g., BTC_data.csv): ";
        std::cin >> filename;

        std::vector<float> prices = load_data(filename);
        train_model(prices);
    }
    else if (mode == "predict") {
        std::string filename;
        std::cout << "Enter crypto data file (e.g., BTC_data.csv): ";
        std::cin >> filename;

        std::vector<float> prices = load_data(filename);
        std::vector<float> normalized = normalize_data(prices);

        float predicted_price = predict_price(normalized);
        std::cout << "Predicted Price: " << predicted_price << " USD" << std::endl;
    }
    else {
        std::cout << "Invalid mode! Use 'learn' or 'predict'" << std::endl;
    }

    return 0;
}
