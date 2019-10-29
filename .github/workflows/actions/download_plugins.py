#!env python
# Download pre-assembled pack of NSIS plugins.

import urllib;
import urllib.request;
import tarfile;
import os;

nsisPluginsUrl = 'https://hifi-content.s3.amazonaws.com/simon/NSIS-hifi-plugins-1.0.tgz';
nsisInstallLocation = 'C:/Program Files (x86)/';

def downLoadPlugins():
    try:
        (filename, headers) = urllib.request.urlretrieve(nsisPluginsUrl);
    except Exception as e:
        print("Can't download " + nsisPluginsUrl);
        print(e);
        return;

    print("Downloaded plugins to " + filename);
    tarObject = tarfile.open(filename, 'r:gz');
    try:
        tarObject.extractall(nsisInstallLocation)
    except Exception as e:
        print("Failed to extract NSIS plugins to " + nsisInstallLocation);
        print(e);
    tarObject.close();

    os.remove(filename);
    return;

downLoadPlugins();

