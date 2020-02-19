# Skystat

Skystat can read observational data from the Sloan Digital Sky survey and the NRAO VLA Sky Survey,
perform various calculations and display the objects on a Mollweide projection,
for further processing with statistical software.

It features
 * Easy to use command-line UI with rapid reaction times
 * POSIX threads for efficient usage of available CPU cores
 * Use of POSIX shared memory to avoid disk bottlenecks

_This was a semester project at the Institute for Astronomy at ETH Zurich._

## Usage

There are no special dependencies required to build skystat.
However, the `gnuplot` package should be installed on the system for plotting.

To build the project, simply run

   ```sh
   $ make
   ```

Then run `skyplot`. Press `h` to get help for available commands.
Catalog data from tge surveys is expected to be located in the `data` subdirectory.

## License

Skystat is distributed under the terms of the GNU General Public License.

See [LICENSE](LICENSE) for details.
