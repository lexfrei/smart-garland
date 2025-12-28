FROM espressif/esp-matter:latest_idf_v5.2.1

# ESP-IDF and ESP-Matter environment variables
ENV IDF_PATH=/opt/esp/idf
ENV ESP_MATTER_PATH=/opt/esp/esp-matter

# Source environment on shell startup
RUN echo 'source $IDF_PATH/export.sh > /dev/null 2>&1' >> ~/.bashrc && \
    echo 'source $ESP_MATTER_PATH/export.sh > /dev/null 2>&1' >> ~/.bashrc

# Set working directory
WORKDIR /workspace
