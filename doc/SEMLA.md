Tool-independent Licensing and Encryption of Modelica Libraries
====

## Scope of this Specification
This draft specification describes a system for distributing proprietary Modelica libraries which supports:

- Encryption

- Licensing

- Secure decryption of encrypted Modelica libraries

- Platform independence
	
It is achieved by:

- a "Modelica Library Containers" format (MLC)

	- containing encrypted __.moc__ files
	
	- an associated _Manifest.xml_ meta-data file
	
- an interface to the MLCs via a "Library Vendor Executable" (LVE)

- using the secured communication "Standardized Encryption of Modelica Libraries and Artifacts" (SEMLA) protocol

An open source project containing:

- this draft specification

- source code examples for

	- implementation of the SEMLA protocol
	
	- LVE code for:
	
		- decryption and licensing of Libraries
		
		- **packagetool**, a utility you can use to create **MLC**s.
	
Open-source Modelica libraries, stored non encrypted, can also be distributed in the MLC format.

### Limitations on the Scope
Note that this specification leaves the details of the LVE implementation undefined, they are at the discretion of the Library Vendor. Any supplied reference implementation is only to give an example and not in any form normative part of this specification. 

## LICENSING AND ENCRYPTION

Licensing, encryption and decryption are **controlled** by the library vendor.

The library vendor chooses if, and how, the library is

- encrypted/decrypted

- licensed

	- through a mechanism chosen by the library vendor
	
	- or through an existing tool’s licensing mechanism
	
- which tool can access the library

- what features are made available to the tool
	
Implementation of the above is **compiled-in** a "Library Vendor Executable" (LVE).

### ENCRYPTION KEYS: SSL and RANDOMIZER KEYS

SSL keys are embedded in the **tool**, the **LVE**s and **packagetool** (after obfuscation with a Randomizer key).

#### SSL keys usage

Tool private key:

- embedded in the **tool**: used to establish a secure communication channel between the tool and the LVE.
	
Tool public key:

- embedded in a **LVE**: used to establish a secure communication channel between the tool and the LVE.

LVE private key:

- embedded in **packagetool**:  used to encrypt proprietary libraries

- embedded in a **LVE**:  used to decrypt proprietary libraries

## LIBRARY VENDOR EXECUTABLE (LVE)

An LVE is a platform specific executable binary which:

- is created by the library vendor

- handles decryption of libraries (packaged in an MLC)

- handles licensing of libraries (packaged in an MLC)

- is packaged within a MLC

- communicates, through the secure SEMLA protocol, with a Modelica tool 

### Platform Specific LVE
 
Four platforms are supported, for each platform a LVE must be compiled. 

Platform and LVE name :

- windows 32 bits: lve_win32.exe

- windows 64 bits: lve_win64.exe

- linux 32 bits: lve_linux32

- linux 64 bits: lve_linux64

And MLC, containing encrypted/licensed libraries, will contain 1 or more LVEs depending on which platforms are supported.

#### Packagetool

If encryption is enabled and an LVE directory exists under the directory where **packagetool** is located:

- LVEs in the _LVE_ directory are copied into the MLC (in the ".library" directory)

- the appropriate fields in the _Manifest.xml_ File are set

## MODELICA LIBRARY CONTAINER

The container is a _zip_ file, with a _.mol_ file extension.

The container has one top-level directory for each contained top-level package (library)

- named and structured according to section 13.2.2 of the Modelica Language Specification version 3.2r2.

- Each top-level directory contains a directory “.library”.

Each “.library” directory contains:

- A _manifest.xml_ file, containing meta-data about the library. 

- If the library is encrypted, there are one or more LVEs, build with the same SSL and Randomizer keys. 

- Additional directories containing any extra files needed by the LVEs or a Modelica tool. The names of each such directory should be the name of the vendor that needs it.

### Packagetool

The SEMLA distribution contains source code for a platform independent utility, which can be used to create MLCs. It **must** be built with the same SSL and Randomizer keys as the LVEs. 
	
If the library is to be encrypted, the **packagetool** will also encrypt the __.mo__ files (to __.moc__) before adding them to the MLC.

### Manifest File

An example of a manifest file _manifest.xml_ can be found at the end of this document. A DTD (Document Type Definition) or an XML schema is specified.

_manifest.xml_ structure, here mandatory fields are highlighted:

- archive
	- manifest
		- version – version of the manifest format (this is version 1.0)
	- library
		- id – (**mandatory**)name of the top-level package
		- enabled – if the library should be loaded by default in the tool
		- title – official title of the library
		- description – description of the library
		- version
			- number – (**mandatory**) the version number
			- build – the build number
			- date – the release date
		- language
			- version – (**mandatory**) the version of the Modelica language that the library uses
		- copyright – textual copyright information
		- license – textual license information
		- encryption – only for encrypted libraries
			- executable – one for each LVE
				- path – relative path from the top-level directory of this library to the executable
				- platform – the platform that this executable should be used for, the values are the same as in the Modelica Language Specification, currently “win32”, “win64”, “linux32”, “linux64”
				- licensing – (default is true) true if this executable can handle licensing of the library
		- icon – an icon to use for the library
			- file – relative path from the top-level directory of this library to an image file containing the icon
		- compatibility – a list of Modelica tools that this library is compatible with
			- tool – (multiple)
				- name – the name of the tool
				- minversion – minimum version of the tool
	- dependencies – libraries that this library depends on
		- dependency – (multiple) 
			- id – name of the top-level package of the library that is the target of the dependency
			- versions – list of compatible versions
				- version – (multiple)
					- number – the version number
					- providers – list of locations that the target library can be downloaded from
						- provider – (multiple)
							- name – human-readable name of provider
							- uri – direct-download URI to the target library

### Library Section

The library part of the manifest file contains the only mandatory fields in the file, which are 'library version' and 'language version'.

Minimal manifest file without encryption, tools or dependencies:

```XML
<?xml version="1.0" encoding="utf-8"?>
<archive>  
  <manifest version="1.0"/>  
  <library id="MyLibrary">
    <version number="1.0"/>
    <language version="3.2"/>    
  </library>
</archive>
```

### Icon Section
```XML
<icon file="Resources/Images/MyIcon.png" />
```

The icon attribute of the manifest file must be set to a path of the icon file, starting from the top-level directory of the library. 

#### Packagetool

The **packagetool** validates that the icon file exists and it also sets the attribute in the manifest file if option -icon is used.

### Compatibility Section

The compatibility section of the manifest is created by reading an xml-file that contains the tools to be added.

Structure of the tools xml file:

```XML
<?xml version="1.0" encoding="utf-8"?>
<compatibility>  
  <tool name="tool 1" minversion="1.0"/>  
  <tool name="tool 2" minversion="3.0"/>  
</compatibility>
```

- the first line in the file is ignored
- the remaining lines are copied into the _manifest.xml_

The **packagetool** does not validate the tools xml file contents.

### Dependencies Section

The dependencies section of the manifest file is created by reading an xml-file that contains the dependencies to be added. 

The structure of the dependencies xml file:

```XML
<?xml version="1.0" encoding="utf-8"?>
<dependencies>  
  <dependency id="depend 1" >  
    <versions>
      <version number="1"/>
        <providers>
          <provider name="Example Company" uri="www.example.com" />
        </providers>
      </version>
    </versions>
  </dependency>
</dependencies>
```

- the first line in the file is ignored
- the remaining lines are copied into the _manifest.xml_

The **packagetool** doesn't validate the tools xml file contents.

## SEMLA - COMMUNICATION PROTOCOL BETWEEN A TOOL AND LVE


```mermaid
sequenceDiagram
    participant Tool
    participant LVE

    rect rgb(191, 223, 255)
        note right of Tool: Handshake<br/>(performed once)
        Tool->>LVE: VERSION <version>
        LVE->>Tool: VERSION <version>
        Tool->>LVE: LIB <path>
        LVE->>Tool: YES
    end
    rect rgb(191, 255, 196)
        note right of Tool: License Check<br/>(performed once)
        Tool->>LVE: FEATURE <feature name>
        LVE->>Tool: YES
        Tool->>LVE: FEATURE <feature name>
        LVE->>Tool: NO
    end
    rect rgb(191, 201, 255)
        note right of Tool:  Get Decrypted File<br/>(used as many times as needed)
        Tool->>LVE: FILE <path>
        LVE->>Tool: FILECONTENT <contents>
    end
```

                              ┌────┐                  ┌───┐                    
                              │Tool│                  │LVE│                    
                              └─┬──┘                  └─┬─┘                    
                                │                       │                      
          ╔═════════════════╤═══╪═══════════════════════╪═════════════════════╗
          ║ PERFORMED ONCE  │   │                       │                     ║
          ╟─────────────────┘   │                       │                     ║
          ║         ╔═══════════╪╤══════════════════════╪═══════════╗         ║
          ║         ║ HANDSHAKE  │                      │           ║         ║
          ║         ╟────────────┘  VERSION <version>   │           ║         ║
          ║         ║           │ ──────────────────────>           ║         ║
          ║         ║           │                       │           ║         ║
          ║         ║           │   VERSION <version>   │           ║         ║
          ║         ║           │ <──────────────────────           ║         ║
          ║         ║           │                       │           ║         ║
          ║         ║           │       LIB <path>      │           ║         ║
          ║         ║           │ ──────────────────────>           ║         ║
          ║         ║           │                       │           ║         ║
          ║         ║           │          YES          │           ║         ║
          ║         ║           │ <──────────────────────           ║         ║
          ║         ╚═══════════╪═══════════════════════╪═══════════╝         ║
          ║                     │                       │                     ║
          ║                     │                       │                     ║
          ║         ╔═══════════╪════╤══════════════════╪═══════════╗         ║
          ║         ║ LICENSE CHECK  │                  │           ║         ║
          ║         ╟────────────FEATURE <feature name> │           ║         ║
          ║         ║           │ ──────────────────────>           ║         ║
          ║         ║           │                       │           ║         ║
          ║         ║           │          YES          │           ║         ║
          ║         ║           │ <──────────────────────           ║         ║
          ║         ║           │                       │           ║         ║
          ║         ║           │ FEATURE <feature nam> │           ║         ║
          ║         ║           │ ──────────────────────>           ║         ║
          ║         ║           │                       │           ║         ║
          ║         ║           │           NO          │           ║         ║
          ║         ║           │ <──────────────────────           ║         ║
          ║         ╚═══════════╪═══════════════════════╪═══════════╝         ║
          ╚═════════════════════╪═══════════════════════╪═════════════════════╝
                                │                       │                      
                                │                       │                      
          ╔═════════════════════╪════════╤══════════════╪═════════════════════╗
          ║ USED AS MANY TIME AS NEEDED  │              │                     ║
          ╟──────────────────────────────┘              │                     ║
          ║         ╔═══════════╪═════════╤═════════════╪═══════════╗         ║
          ║         ║ GET DECRYPTED FILE  │             │           ║         ║
          ║         ╟──────────────────FILE <path>      │           ║         ║
          ║         ║           │ ──────────────────────>           ║         ║
          ║         ║           │                       │           ║         ║
          ║         ║           │ FILECONTENT <contents>│           ║         ║
          ║         ║           │ <──────────────────────           ║         ║
          ║         ╚═══════════╪═══════════════════════╪═══════════╝         ║
          ╚═════════════════════╪═══════════════════════╪═════════════════════╝
                              ┌─┴──┐                  ┌─┴─┐                    
                              │Tool│                  │LVE│                    
                              └────┘                  └───┘                    

This protocol defines : 
- the establishment a secure communication channel between the tool and the LVE (see Handshake) 
	- the encrypted communication is done through the LVE's stdin and stdout
- the agreement over which version of the SEMLA protocol to use 
- the decryption of Modelica code and/or licensing. 

## MESSAGE FORMAT

### Encoding

Messages are in 8-bit ASCII

### Simple message with no arguments

Format: "\<COMMAND>LF"
	
- \<COMMAND> - command name, in all caps
- “LF” - line feed character
		
Messages using this form: 
	
- “LICENSEINFO”
- “NOTSIMPLE”
- “TOOLS”
- “YES”

### Message with decimal integer data

Format: "\<COMMAND> <number>LF"
	
- \<COMMAND> - command name in all caps
- \<number> - a decimal number – 32-bit signed integer, string, base 10 representation
- “LF” - line feed character

Messages using this form: 
	
- “VERSION”

### Message with variable length data

Format: "\<COMMAND> \<length>LF\<data>"

- \<COMMAND> - command name in all caps
- \<length> - data length in bytes – 32-bit signed integer, string, base 10 representation
- “LF” - line feed character
- \<data> - data bytes

Messages using this form: 

- “FEATURE”
- “FILE”
- "FILECONT”
- “LIB”
- “LICENSE”
- “NO”
- “PUBKEY”
- “RETURNFEATURE”
- “RETURNLICENSE”
- “TEXT”
- “TOOLLIST”

### Message with an integer number and a variable length data

Format: "\<COMMAND> \<number> \<length>LF\<data>"

- \<COMMAND> - command name in all caps
- \<number> - 32-bit signed integer, string, base 10 representation
- \<length> - data length in bytes – 32-bit signed integer, string, base 10 representation
- “LF” - line feed character
- \<data> - data bytes

Messages using this form:

- “ERROR”
		
### Error Handling

The LVE can respond “ERROR \<error code> \<error message>” at any time. 

Standard error code:

- \<error code> is one of the following:

	- 1 - Command not understood
	
	- 2 - Too low version of the protocol – only after message VERSION
	
	- 3 - The LVE doesn’t handle licensing – only after message FEATURE or LICENSE
	
	- 4 - File not found – only after message FILE or LIB
	
	- 5 - This tool is not allowed to decrypt that file – only after message FILE
	
	- 6 - File I/O error
	
	- 7 - License error
	
	- 8 - Other error
	
- \<error message> - an error message suitable for display

### Authentication

The LVE contains a list of the public keys of the tools that it trusts. It will only connect to those tools.

The tools implicitly trusts the LVE as only the LVE can provide licenses and decryption of the MLC.

### Handshake

- 1 - the tool initiates a cryptographic handshake according to TLS 1.2.

- 2 - the tool sends the highest version of the protocol it supports (integer): “VERSION \<version>”

- 3 - the LVE responds with the highest version of the protocol it supports: “VERSION \<version>”
	- If tool-version is less than the LVE-version, then the reply is: “NO \<reason>”.

- 4 - the tool sends a path: “LIB \<path>”
	- If only the “.library” folder was extracted from the MLC, "\<path>" is the path to the library archive
		
	- If the entire contents of the MLC was extracted, "\<path>" is the path to the top-level directory of the library

- 5 - the LVE responds: “YES”.

### License check

#### Check for a feature

- 1 - the tool sends: “FEATURE \<feature name>”
- 2 - the LVE responds: “YES” or “NO \<reason>”

#### Simplified license check

The LVE may **optionally** allow for a simplified license check.

In the simplified check, tool only asks for permission for the user to use an entire top-level package contents. 
 
- 1 - the tool sends: “LICENSE \<package name>”
- 2 - the LVE answers one of:
	- “YES”
	- “NO \<reason>” 
	- “NOTSIMPLE” in case simplified license check is not supported.

### Authentication

The LVE embeds a list of tool public keys that it trusts.

The Tool's public key is compared against the list of trusted keys during the TLS handshake. 

### Return of licenses

#### Return a feature

- Tool sends – “RETURNFEATURE \<feature name>”
- LVE answers – “YES”

#### Return a simplified license

- Tool sends – “RETURNLICENSE \<package name >”
- LVE answers - “YES”

### Decryption

The tool can request a decrypted library file from the LVE using the "FILE" command.

The LVE:

- can decide to refuse, or accept, "FILE" commands at any time.
- interprets paths relative to the top level package of the library
- uses '/' as the path separator
- ignores special path elements '.' and '..' which may not be used
- accepts path to directories ending with '/' or not
 
Getting the contents of a file:

- Tool sends – “FILE \<path>”
- LVE answers - “FILECONT \<content>”

### General information Query

The tool may, after the cryptographic handshake, query the LVE for general information.

#### List of supported tools

- Tool sends – “TOOLS”
- LVE answers: “TOOLLIST \<list>” where "\<list>" is:
	- a sequence of Modelica tools in the formatted as:
		- one line for each tool terminated by the line feed character
		- each line formated as: "tool-public-key name-of-the-tool"
			- tool-public-key: a hexadecimal number string without a prefix (i.e. no “0x”)

#### License information

- Tool sends– “LICENSEINFO”
- LVE answers - “TEXT \<info>”

"\<info>" can be used to solving license/configuration problems.

## INSTALLING MODELICA LIBRARY CONTAINER

A library can be installed by the tool:

- by extracting only the “.library” folder from the library
- by extracting entire directory structure of the library from the MLC.
	- the directory structure, and file names, of the library must be the same as in the MLC
	- the library can be placed in any location (tool decision) 
		- the library can be renamed (tool decision)

We recommend that:

- the library is placed in a directory on the MODELICAPATH
- named “PACKAGENAME” or “PACKAGENAME VERSION” (as per the Modelica Language Specification)

### Reading open-source library

Reading a non-encrypted library is equivalent to reading a library stored on disk (as per the Modelica Language Specification).

### Reading an encrypted library

- from the __manifest.xml__, in the “.library” directory, read the **LVE**'s for the current platform.
- start the platform specific **LVE**
- communicate with LVE through its stdin & stdout
- encrypted Modelica files (__.moc__) are read through the LVE
- non-encrypted Modelica files can also be read through the LVE or directly from disk

## APPENDIX

### Building SEMLA

#### SSL and Randomizer keys

As LVEs are created in different builds (one or each platform), you need to use the **same** SSL keys and Randomizer Key in **every** build to create a MLC that is platform independent.

if no keys are provided, only for testing purpose, SSL and RANDOMIZER keys are generated.

### Example “manifest.xml” file for an encrypted library:

```XML
<?xml version="1.0" encoding="utf-8"?>
<archive>
  <!-- All paths in the file are interpreted as relative to the directory of the top-level 
       package (e.g., the path to this file would be ".library/manifest.xml"), and allows 
       only forward slashes as directory separators. -->
  <manifest version="1.0"/>
  <!-- The id attribute is the actual Modelica identifier of the library. The file attribute
       from SMA is no longer needed, as it will always be "package.mo" or "package.moc" 
       (and the tool will need the logic of checking for both .mo and .moc anyway).
       The enabled attribute (optional, default value is true) indicates whether the library
       should be enabled/loaded by default. -->
  <library id="ExampleLib" enabled="true">
    <!-- Official title of the library (optional) -->
    <title>Example Encrypted Library</title>
    <!-- Description (optional) -->
    <description>
      Dummy library showing directory structure for an encrypted library (with empty files)
    </description>
    <!-- The version of the library. Version information is formatted according to the
         Modelica language specification. The build and date attributes are optional. -->
    <version number="1.0" build="1" date="2013-08-04"/>
    <!-- Version of the Modelica language that is used in this library. -->
    <language version="3.2" />
    <!-- Copyright notice (optional)-->
    <copyright>
      Copyright © 2014, Modelon AB.
    </copyright>
    <!-- License information (optional) -->
    <license>
      Some license information.
    </license>
    <!-- Encryption/license check information (only for proprietary libraries).
         If this is present, then the library is encrypted. -->
    <encryption>
      <!-- Library vendor executable. May be repeated - one for each supported platform. 
             path:       the path to the executable
             platform:   the OS/platform to use this executable on, must be unique among 
                         executable nodes
             licensing:  if this executable handles licensing (optional, default is true)
           The executable shall be placed under the vendor-specific directory 
           (i.e. .library/VENDORNAME/). The normal case is that licensing has the same 
           value for each executable node, but it is allowed to have different values 
           for different platforms. -->
      <executable path=".library/Modelon/vendor.exe" platform="win32" licensing="true" />
      <executable path=".library/Modelon/vendor32" platform="linux32" licensing="true" />
      <executable path=".library/Modelon/vendor64" platform="linux64" licensing="true" />
    </encryption>
    <!-- Icon for the library (PNG format) (optional) -->
    <icon file="Resources/Images/el.png" />
  </library>
  <!-- Leaving out optional compatibility and dependencies in this example. -->
</archive>
```

### System Requirements for commercial libraries implemented in this proposal

#### The Library Vendor

- Needs to define which libraries should be made available to which tools, e.g.: explicit encoding of supported tools. This is achieved through a list of trusted tool keys.

- Needs to enable/disable parts (features) in any given library based on standardized annotations. Achieved through standard license annotations.

- Needs to specify the visibility of components based on the standardized annotations. Achieved through standard license annotations.

- Need to check the license for a whole library without parsing the Modelica code. Achieved optionally, through an alternate licensing mechanism for tools that do not understand license annotations.

- Needs to license/protect external libraries (e.g.: with C and F77) independently of the licensing mechanism for the library.

- Needs to make releases, of existing and new libraries, without any action from the tool vendor.

- Needs an encryption mechanism to ensure protection of library vendor IP (the Modelica source code).

- Needs an encryption mechanism which can be used with or without a licensing mechanism.

- Can implement a custom licensing mechanism.

#### Tool 

- Storage of Modelica source code on disc in an unencrypted form is not allowed.

- The Tool supports standardized mechanisms for enabling/disabling parts of a library, by means of annotations defined in the Modelica specification.

- Well documented low level API for licensing and encryption.

- Possible customizations of licensing and encryption modules.

- Can display Error messages for errors happening on the LVE side.

- Alternative, tool specific, licensing mechanisms are possible.

- Enables the creation of containers which include multiple libraries/top-level packages, in a single file.

- May have more than one key pair. For tools that have more than one key pair, the LVE needs to be restarted in between trying different key pairs.
	
#### User

- The API should make installation procedures for libraries convenient. This is possible with tool support

- The library is distributed as a single file with a well-defined file extension.

- Error messages enable users to isolate errors related to licensing and encryption.

## Vulnerability

### Analysis

Consequences if a cryptographic key is extracted or altered: 

- Session key (generated for each session)
	- Extraction: allows eavesdropping on the communication.
	- Alteration: not possible, would break the communication between the tool and the LVE.
	
- Tool private key 
	- Extraction: allows decryption of all libraries that trust the key.
	- Alteration: LVEs will stop trusting the key and no communication would be possible.
	
- LVE trusted keys list
	- Extraction: not useful to attacker.
	- Alteration: adding the attacker key to the list allows the decryption of the library.

- LVE key used to decrypt the library
	- Extraction: allows the decryption the library.
	- Alteration: not useful to attacker, makes it impossible for the LVE to decrypt libraries.
		
The most serious breach is an attacker obtaining the tools private key; it would allow the decryption of any library released for that tool.
		
These vulnerabilities exists in any tool that supports encrypted libraries and embed encryption keys.

### Mitigation

Encryption keys embedded in the **tool**, the **LVEs**, and **packagetool** binaries must be protected with an obfuscation scheme.

**Change the obfuscation code to be unique for your company**.

For **each** release of libraries:

- change the obfuscation code if possible

- **each** library gets a new randomly generated SSL key set and Randomized Key

	- a new set of **LVEs**
	
	- a new **packagetool**
