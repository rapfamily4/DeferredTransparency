#ifndef TEXTURE_HPP
#define TEXTURE_HPP


/* 2D texture class with utilities */
class Texture {
	public:		
		// Constructor
		Texture(bool alpha = false);
		
		// Generates texture object from loaded image
		void generate(unsigned int width, unsigned int height, unsigned char* data);
		
		// Generates mipmaps from texture object
		void generateMipmaps();
		
		// Bind texture object to GL_TEXTURE_2D target
		void bind() const;
		
		// Bind texture object to GL_TEXTURE_2D target and activate the specified texture unit
		//void bind(unsigned int unit) const;
		
		// Getters
		unsigned int getID();
		unsigned int getWidth();
		unsigned int getHeight();
		unsigned int getTextureFormat();
		unsigned int getLoadedFormat();
		unsigned int getWrapS();
		unsigned int getWrapT();
		unsigned int getFilterMin();
		unsigned int getFilterMag();
		
		// Setters
		void setWrapS(int wrapS);
		void setWrapT(int wrapT);
		void setFilterMin(int filterMin);
		void setFilterMag(int filterMag);
	
	private:
		// State
		bool m_generatedTexture;
		bool m_generatedMipmaps;
		
		// Texture ID
		unsigned int m_id;
		
		// Texture image dimensions
		unsigned int m_width, m_height;	// Dimesions of loaded image
		
		// Texture format
		unsigned int m_textureFormat;	// Format of texture object
		unsigned int m_loadedFormat;	// Format of loaded image
		
		// Texture configuration
		unsigned int m_wrapS;			// Wrapping mode on S axis
		unsigned int m_wrapT;			// Wrapping mode on T axis
		unsigned int m_filterMin;		// Filtering mode if texture pixels < screen pixels
		unsigned int m_filterMag;		// Filtering mode if texture pixels > screen pixels
};


#endif