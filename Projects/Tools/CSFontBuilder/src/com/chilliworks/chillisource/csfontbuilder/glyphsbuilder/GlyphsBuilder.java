/**
 * GlyphsBuilder.java
 * Chilli Source
 * Created by Ian Copland on 09/10/2014.
 * 
 * The MIT License (MIT)
 * 
 * Copyright (c) 2014 Tag Games Limited
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

package com.chilliworks.chillisource.csfontbuilder.glyphsbuilder;

import com.chilliworks.chillisource.csfontbuilder.Glyphs;
import com.chilliworks.chillisource.toolutils.Logging;
 
/**
 * A static class containing methods for converting a vector font (TTF or OTF) to
 * bitmap font glyphs with kerning info.
 * 
 * @author Ian Copland
 */
public final class GlyphsBuilder
{
	/**
	 * Converts the font described in the input options to bitmap font glyphs. Also
	 * outputs kerning info for the glyphs.
	 * 
	 * @author Ian Copland
	 * 
	 * @param in_options - The builder options.
	 * 
	 * @return Whether or not the builder succeeded.
	 */
	public static boolean build(GlyphsBuilderOptions in_options)
	{
		Glyphs glyphs = GlyphsRenderer.render(in_options);
		if (glyphs == null)
		{
			Logging.logFatal("Failed to render glyphs for font: " + in_options.m_fontName);
			return false;
		}
		
		if (GlyphsWriter.write(glyphs, in_options.m_outputDirectoryPath) == false)
		{
			Logging.logFatal("Failed to save glyphs to directory: " + in_options.m_outputDirectoryPath);
			return false;
		}
		
		return true;
	}	

}
