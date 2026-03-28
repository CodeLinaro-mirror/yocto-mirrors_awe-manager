# AWE AWC: Behavioral View

During initialization the awe_AWC reads and parses the {{name.awe_awc_db}} index file. After reading that information data is stored in memory for later usage/lookup. 

All awe_AWC API methods are strictly synchronous. Every call results in an immediate lookup of information in memory and returns a possible error code or result data.
