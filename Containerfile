FROM docker.io/espressif/esp-matter:latest_idf_v5.2.1

ENV IDF_PATH=/opt/esp/idf
ENV ESP_MATTER_PATH=/opt/esp/esp-matter

RUN echo 'source $IDF_PATH/export.sh > /dev/null 2>&1' >> ~/.bashrc && \
    echo 'source $ESP_MATTER_PATH/export.sh > /dev/null 2>&1' >> ~/.bashrc

WORKDIR /workspace
