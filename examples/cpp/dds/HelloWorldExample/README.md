# Fast DDS HelloWorld example

This example provides a very simple application which can be run as either `publisher` or `subscriber` to send or receive *HelloWorld* messages.

1. [Build the example](#build-the-example)
1. [Run the example](#run-the-example)

## Build the example

There are several ways in which this example can be built.

### Build alongside Fast DDS

To build and install all the examples while building Fast DDS, make sure to pass both the `-DCOMPILE_EXAMPLES=ON` and `-DINSTALL_EXAMPLES=ON` options to CMake.

### Build standalone

From a terminal containing `fastrtps-config.cmake` in the `$PATH` and `libfastrtps` in the `$LD_LIBRARY_PATH`, run:

```bash
cd <this directory>
mkdir build && cd build
cmake ..
cmake --build .
```

## Run the example

* **IMPORTANT**: In order to run the example, `libfastrtps` must be in the `$LD_LIBRARY_PATH` if Fast DDS was built as a shared library.
* **IMPORTANT**: Mind the `DDSHelloWorldExample` executable generated during the building process in located:

    1. Directly on the root of the `build` directory when running the example from the `build` directory
    1. Under `<path to example install>/bin` when running from the installation directory

This example provides a simple CLI.
To see all the possible arguments, move to the appropriate directory and run:

```bash
./DDSHelloWorldExample --help
```

### Simple publisher and subscriber

In one terminal and run:

```bash
./DDSHelloWorldExample subscriber
```

In a second terminal and run:

```bash
./DDSHelloWorldExample publisher
```

### Load QoS from XML configuration file

In order to use xml profiles (`--env` or shorcut `-e` cli flags):

1. Reference the xml profiles file setting the environment variable `FASTRTPS_DEFAULT_PROFILES_FILE` to its path.
2. name it DEFAULT_FASTRTPS_PROFILES.xml and make sure the file is besides the DDSHelloWorldExample binary.

The profile loaded will be the mark as default one with the corresponding attribute.
For example, the following configuration will create a participant called `Example dummy name` and modify the endpoint set up.

**Note**: the `profile_name` attribute is mandatory.

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles">
    <participant profile_name="name_is_mandatory" is_default_profile="true">
        <rtps>
            <name>Profiles example name</name>
        </rtps>
    </participant>
    <data_writer profile_name="datawriter_name_is_mandatory" is_default_profile="true">
        <qos>
            <reliability>
                <kind>BEST_EFFORT</kind>
            </reliability>
        </qos>
    </data_writer>
    <data_reader profile_name="datareader_name_is_mandatory" is_default_profile="true">
        <topic>
            <historyQos>
                <kind>KEEP_LAST</kind>
                <depth>5</depth>
            </historyQos>
        </topic>
    </data_reader>
</profiles>
```