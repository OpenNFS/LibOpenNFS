#pragma once

/**
 * FSH/QFS Archive Library
 *
 * Modern C++ implementation for reading and writing EA's FSH texture archives
 * and QFS compressed files, as used in Need for Speed games.
 *
 * Original fshtool by Denis Auroux (1998-2002)
 * C++ port for OpenNFS project
 *
 * Usage example:
 *
 *   #include <Common/Fsh/Fsh.h>
 *
 *   using namespace LibOpenNFS::Shared;
 *
 *   // Load a QFS/FSH archive
 *   FshArchive archive;
 *   if (archive.Load("track0.qfs")) {
 *       // Extract all textures to a directory
 *       archive.ExtractAll("output/textures");
 *
 *       // Or access individual textures
 *       for (const auto& tex : archive.Textures()) {
 *           std::cout << "Texture: " << tex.Name()
 *                     << " (" << tex.Width() << "x" << tex.Height() << ")\n";
 *
 *           // Get pixel data as 32-bit ARGB
 *           auto pixels = tex.ToARGB32();
 *       }
 *
 *       // Find texture by name
 *       if (const auto* tex = archive.GetTexture("sky0")) {
 *           tex->ExportToBmp("sky.bmp");
 *       }
 *   }
 *
 * Supported pixel formats:
 *   - 8-bit indexed (with palette)
 *   - 16-bit RGB (5:6:5)
 *   - 16-bit ARGB (1:5:5:5)
 *   - 16-bit ARGB (4:4:4:4)
 *   - 24-bit RGB
 *   - 32-bit ARGB
 *   - DXT1 compressed
 *   - DXT3 compressed (with alpha)
 */

#include "FshArchive.h"
#include "FshTexture.h"
#include "FshTypes.h"
#include "QfsCompression.h"