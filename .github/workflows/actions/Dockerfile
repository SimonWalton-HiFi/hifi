FROM ubuntu:18.04
MAINTAINER DevOps Team (devops@highfidelity.io)

EXPOSE 40100 40101 40102
EXPOSE 40100/udp 40101/udp 40102/udp
EXPOSE 48000/udp 48001/udp 48002/udp 48003/udp 48004/udp 48005/udp 48006/udp

RUN apt-get update && apt-get install -y software-properties-common apt-utils
RUN apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 15FF1AAE && add-apt-repository "deb http://debian.highfidelity.com stable main"
RUN echo UTC >/etc/timezone
# Installing via dependency causes interactive hang:
RUN apt-get -y install tzdata
RUN apt-get update && apt-get -y --allow-unauthenticated install libglib2.0-0 libgl1-mesa-dev gdb python-pip build-essential \
    openssl libssl-dev libssl1.0.0 unzip flex bison gperf perl libsqlite3-dev \
    libfontconfig1-dev libicu-dev libfreetype6 libssl-dev libpng-dev libjpeg-dev \
    python libx11-dev libxext-dev libtbb2 hifiqt5.12.3 \
    ntp unzip libwww-perl libdatetime-perl make pkg-config libnss3 libxi6 \
    libxcursor1 libxcomposite1 libasound2 libxtst6 libxslt1.1 supervisor

RUN mkdir -p /etc/hifi/server/plugins /etc/hifi/server/resources /etc/hifi/server/imageformats/ && \
    ln -s /usr/local/Qt5.12.3/5.12.3/gcc_64/plugins/imageformats/* /etc/hifi/server/imageformats/
COPY ./assignment-client /etc/hifi/server/
#COPY ./oven /etc/hifi/server/
COPY ./domain-server /etc/hifi/server/
COPY ./plugins/hifiCodec/libhifiCodec.so /etc/hifi/server/plugins/
COPY ./plugins/pcmCodec/libpcmCodec.so /etc/hifi/server/plugins/
COPY ./*.so /lib/
RUN ln -sf /lib/libquazip5.so /lib/libquazip5.so.1
#COPY ./resources/ /etc/hifi/server/resources/
#COPY ./supervisor/hifi.conf /etc/supervisor/conf.d/hifi.conf
RUN for fn in /usr/local/Qt5.12.3/5.12.3/gcc_64/plugins/imageformats/*.so; do \
    if [ ! -x $fn ]; then ln -s $fn /etc/hifi/server/imageformats; fi; done
RUN chmod +x /etc/hifi/server/domain-server
RUN chmod +x /etc/hifi/server/assignment-client

# Ensure `domain-server` and `assignment-client` execute.
RUN /etc/hifi/server/domain-server --version > /etc/hifi/server/version && \
    /etc/hifi/server/assignment-client --version >> /etc/hifi/server/version

CMD ["/usr/bin/supervisord", "-c", "/etc/supervisor/conf.d/hifi.conf"]
