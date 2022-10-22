#include <iostream>

#include <glad/glad.h>

#include "texture.hpp"


/* CONSTRUCTOR */
Texture::Texture(bool alpha)
    : m_generatedTexture{false}, m_generatedMipmaps{false}, m_width(0), m_height(0), m_textureFormat(alpha ? GL_RGBA : GL_RGB), m_loadedFormat(alpha ? GL_RGBA : GL_RGB), m_wrapS(GL_REPEAT), m_wrapT(GL_REPEAT), m_filterMin(GL_LINEAR), m_filterMag(GL_LINEAR) {
    glGenTextures(1, &m_id);
}


/* PUBLIC FUNCTIONS */
void Texture::generate(unsigned int width, unsigned int height, unsigned char* data) {
	if (!m_generatedTexture) {
		m_width = width;
		m_height = height;

		// Create texture object
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexImage2D(GL_TEXTURE_2D, 0, m_textureFormat, width, height, 0, m_loadedFormat, GL_UNSIGNED_BYTE, data);

		// Set texture filering and wrapping modes
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filterMin);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filterMag);

		// Unbind
		glBindTexture(GL_TEXTURE_2D, 0);

		// State that the texture has been generated
		m_generatedTexture = true;
	}
}

void Texture::generateMipmaps() {
	if (m_generatedTexture && !m_generatedMipmaps) {
		// Bind to GL_TEXTURE_2D target
		glBindTexture(GL_TEXTURE_2D, m_id);

		// Generate mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);

		// Unbind
		glBindTexture(GL_TEXTURE_2D, 0);

		// State that the mipmaps have been generated
		m_generatedMipmaps = true;
	}
}

void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, m_id);
}

/*void Texture::bind(unsigned int unit) const {
	int maxTextureImageUnits;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);
	if (0 <= unit && unit < maxTextureImageUnits)
		glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, m_id);
}*/

unsigned int Texture::getID() {
	return m_id;
}

unsigned int Texture::getWidth() {
	return m_width;
}

unsigned int Texture::getHeight() {
	return m_height;
}

unsigned int Texture::getTextureFormat() {
	return m_textureFormat;
}

unsigned int Texture::getLoadedFormat() {
	return m_loadedFormat;
}

unsigned int Texture::getWrapS() {
	return m_wrapS;
}

unsigned int Texture::getWrapT() {
	return m_wrapT;
}

unsigned int Texture::getFilterMin() {
	return m_filterMin;
}

unsigned int Texture::getFilterMag() {
	return m_filterMag;
}

void Texture::setWrapS(int wrapS) {
	m_wrapS = wrapS;
	if (m_generatedTexture) {
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void Texture::setWrapT(int wrapT) {
	m_wrapT = wrapT;
	if (m_generatedTexture) {
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void Texture::setFilterMin(int filterMin) {
	m_filterMin = filterMin;
	if (m_generatedTexture) {
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMin);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void Texture::setFilterMag(int filterMag) {
	m_filterMag = filterMag;
	if (m_generatedTexture) {
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMag);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}