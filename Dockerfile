FROM fncs/zeromq

ENV FNCS_PORT=5570
ENV FNCS_LOG_FILE=yes
ENV FNCS_LOG_STDOUT=yes
ENV FNCS_LOG_TRACE=yes
ENV FNCS_LOG_LEVEL=DEBUG4
ENV FNCS_CONFIG_FILE=""
ENV FNCS_NAME=""
ENV FNCS_BROKER=tcp://*:${FNCS_PORT}

COPY . ${TEMP_DIR}/fncs_src

WORKDIR ${TEMP_DIR}/fncs_src
RUN LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${FNCS_INSTALL}/lib
RUN ./configure --prefix=${FNCS_INSTALL} --with-zmq=${FNCS_INSTALL}
RUN make
RUN make install

WORKDIR /fncs

COPY run_broker.sh /fncs

EXPOSE ${FNCS_PORT}

RUN printenv
ENTRYPOINT ["./run_broker.sh &>fncs.log"]
