# AWE AWC: Resource View

awe_AWC does not persist data, but it reads information from {{name.awe_awc_db}}. More specifically, it reads an index file and keeps the data of that file in memory for fast lookups.  

## AWC Format

Currently, AWC is simply stored as a "folder" or file structure on flash file system.

        some/path_to_awc/awc_index.txt
                         set_get.awb
                         preset_1.awb
                         preset_carplay.awb
                         preset_carline_XYZ.awb

The index file `awc_index.txt` contains the (meta) data about the installed signal flow files. The signal flow files consist of a "main" AWB file (`set_get.awb` in the example above) and -optionally- one or more preset AWB files. 

While the "main" AWB file typically is applied directly after system startup, the preset AWB data is selected and applied by applications at any time. The preset AWB files only modify the existing/running {{name.awe_sf}}, hence it must be ensured the stored AWB files "belong to each other". 

## Custom Fields in AWC Index File

Currently, the file format of `awc_index.txt` is kept super simple and basically has a CSV like format. Theoretically, the file can be created by hand, but it should be generated using so-called AWC tooling. 

!!! info
    To get AWC tooling installed, one needs to install a few Python packages. Details about how to do that are documented outside the scope of this document. 

These custom items are currently available, and may be helpful for debugging/tracing. They are stored as simple strings.

```text
_INFO_,version,1.2.3-version
_INFO_,date,1970-01-01
_INFO_,description,This is an AWC file
```

