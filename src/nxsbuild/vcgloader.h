#ifndef VCGLOADER_H
#define VCGLOADER_H

#include "meshloader.h"
#include "wrap/io_trimesh/import.h"

template <typename Mesh> class VcgLoader: public MeshLoader {
public:
	Mesh *mesh = nullptr;
	bool per_face_texture = false;
	typedef typename Mesh::FaceType FaceType;
	typedef typename Mesh::VertexType VertexType;

	VcgLoader(QString filename) {
		Mesh *m = new Mesh;
		int loadmask = 0;
		if(vcg::tri::io::Importer<Mesh>::Open(*m, filename.toStdString().c_str(), loadmask)!=0)
			throw QString("Failed loading file: "  + filename);
		has_colors = loadmask | vcg::tri::io::Mask::IOM_VERTCOLOR;
		has_normals = loadmask | vcg::tri::io::Mask::IOM_VERTNORMAL;
		has_textures = (loadmask | vcg::tri::io::Mask::IOM_VERTTEXCOORD);
		if(loadmask | vcg::tri::io::Mask::IOM_WEDGTEXCOORD)
			has_textures = per_face_texture = true;
		for(auto s: m->textures)
			texture_filenames.push_back(s.c_str());
		load(m, has_colors, has_normals, has_textures);
	}

	void load(Mesh *m, bool colors, bool normals, bool textures) {
		mesh = m;
		has_colors = colors;
		has_normals = normals;
		has_textures = textures;

		//fill in the texture filenames
	}
	void setMaxMemory(quint64 max_memory) {}

	quint32 nVertices() { mesh->nv; }
	quint32 nTriangles() { mesh->nf; }

	quint32 getTriangles(quint32 size, Triangle *buffer) {
		int count = 0;
		while(current_face	< mesh->face.size() && count < size) {
			FaceType &face = mesh->face[current_face];
			current_face++;
			if(face.IsD())
				continue;

			Triangle &triangle = buffer[count];
			for(int k = 0; k < 3; k++) {
				VertexType *vp = face.V(k);
				Vertex &vertex = triangle.vertices[k];
				for(int j = 0; j < 3; j++) {
					vertex.v[j] = vp->P()[j];
				}
				if(has_normals) {
					for(int j = 0; j < 4; j++) {
						vertex.c[j] = vp->C()[j];
					}
				}
				if(has_textures) {
					auto t = per_face_texture ? face.WT(k) : vp->T();
					vertex.t[0] = t.U();
					vertex.t[1] = t.V();
				}

//				unsigned char c[4]; //colors
//				float t[2]; //texture
			}
			count++;
		}
		return count;
	}

	quint32 getVertices(quint32 size, Splat *vertex) {
	}
private:
	int current_face = 0;
	int current_vertex = 0;
};

#endif // VCGLOADER_H