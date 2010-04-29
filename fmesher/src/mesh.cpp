#include <cstddef>
#include <cstring>
#include <set>
#include <map>
#include <sstream>
#include <cmath>

#include "predicates.h"

#include "mesh.h"


namespace fmesh {




  class Xtmpl {
  private:
    int window_;
    char* name_char_;
    int sx_, sy_;
    double minx_, maxx_, miny_, maxy_;
    bool draw_text_;
  public:
    Xtmpl(const Xtmpl& X)
      : window_(X.window_+1), name_char_(NULL),
	sx_(X.sx_), sy_(X.sy_),
	minx_(X.minx_), maxx_(X.maxx_),
	miny_(X.miny_), maxy_(X.maxy_), draw_text_(true) {
      open(std::string(X.name_char_),X.sx_,X.sy_);
      setAxis(X.minx_, X.maxx_, X.miny_, X.maxy_);
    };
    Xtmpl(bool draw_text, int sx, int sy,
	  double minx,
	  double maxx,
	  double miny,
	  double maxy,
	  std::string name = "fmesher::Mesh")
      : window_(-1), name_char_(NULL),
	sx_(sx), sy_(sy),
	minx_(minx), maxx_(maxx),
	miny_(miny), maxy_(maxy), draw_text_(draw_text) {
      open(name,sx_,sy_);
      setAxis(minx, maxx, miny, maxy);
    };
    void reopen(int sx, int sy) {
      if (!(window_<0))
	close();
      window_ = 0;
      sx_ = sx;
      sy_ = sy;
      xtmpl_window = window_;
      xtmpl_open(sx_,sy_,name_char_);
    };
    void reopen(int sx, int sy, bool draw_text) {
      reopen(sx,sy);
      draw_text_ = draw_text;
    };
    void open(std::string name,
	      int sx, int sy) {
      if (!(window_<0))
	close();
      window_ = 0;
      sx_ = sx;
      sy_ = sy;
      if (name_char_) delete[] name_char_;
      name_char_ = new char[name.length()+1];
      name.copy(name_char_,name.length(),0);
      name_char_[name.length()] = '\0';
      xtmpl_window = window_;
      xtmpl_open(sx,sy,name_char_);
      setAxis(-0.05,1.05,-0.05,1.05);
    };
    void close() {
      if (window_<0)
	return;
      xtmpl_window = window_;
      xtmpl_close();
      window_ = -1;
    };
    ~Xtmpl() {
      close();
      if (name_char_) delete[] name_char_;
    };

    void clear() {
      xtmpl_window = window_;
      xtmpl_clear();
    }

    void setSize(int sx, int sy) {
      reopen(sx,sy);
    };
    void setAxis(double minx, double maxx,
		 double miny, double maxy) {
      clear();
      minx_ = minx;
      maxx_ = maxx;
      miny_ = miny;
      maxy_ = maxy;
    };

    void arc(const double* s0, const double* s1);
    void line(const double* s0, const double* s1);
    void text(const double* s0, std::string str);

  };

  void Xtmpl::arc(const double* s0, const double* s1)
  {
    int n = 10;
    xtmpl_window = window_;
    double p0[2];
    double p1[2];
    double s[3];
    double l;
    int dim;
    p1[0] = s0[0];
    p1[1] = s0[1];
    for (int i=1;i<=n;i++) {
      l = 0.0;
      p0[0] = p1[0]; p0[1] = p1[1];
      for (dim=0;dim<3;dim++) {
	s[dim] = ((n-i)*s0[dim]+i*s1[dim])/n;
	l += s[dim]*s[dim];
      }
      l = std::sqrt(l);
      for (dim=0;dim<2;dim++)
	p1[dim] = s[dim]/l;
      
      xtmpl_draw_line((int)(sx_*(p0[0]-minx_)/(maxx_-minx_)),
		      (int)(sy_*(p0[1]-miny_)/(maxy_-miny_)),
		      (int)(sx_*(p1[0]-minx_)/(maxx_-minx_)),
		      (int)(sy_*(p1[1]-miny_)/(maxy_-miny_)));
    }
  };
  void Xtmpl::line(const double* s0, const double* s1)
  {
    xtmpl_window = window_;
    xtmpl_draw_line((int)(sx_*(s0[0]-minx_)/(maxx_-minx_)),
		    (int)(sy_*(s0[1]-miny_)/(maxy_-miny_)),
		    (int)(sx_*(s1[0]-minx_)/(maxx_-minx_)),
		    (int)(sy_*(s1[1]-miny_)/(maxy_-miny_)));
  };
  void Xtmpl::text(const double* s0, std::string str)
  {
    if (!draw_text_) return;
    char* str_ = new char[str.length()+1];
    str.copy(str_,str.length(),0);
    str_[str.length()] = '\0';
    xtmpl_window = window_;
    xtmpl_text((int)(sx_*(s0[0]-minx_)/(maxx_-minx_)),
	       (int)(sy_*(s0[1]-miny_)/(maxy_-miny_)),
	       str_,str.length());
    delete[] str_;
  };





  


  /*
    T-E+V=2
    
    closed 2-manifold triangulation:
    E = T*3/2
    T = 2*V-4

    simply connected 2-manifold triangulation:
    T <= 2*V-5

  */

  Mesh::Mesh(Mtype manifold_type,
	     size_t V_capacity,
	     bool use_VT,
	     bool use_TTi) : type_(manifold_type),
			     Vcap_(V_capacity), Tcap_(2*V_capacity),
			     nV_(0), nT_(0),
			     use_VT_(use_VT), use_TTi_(use_TTi)
  {
    if (Vcap_ > 0) {
      TV_ = new int[Vcap_][3];
      TT_ = new int[Tcap_][3];
      if (use_VT_)
	VT_ = new int[Vcap_];
      else
	VT_ = NULL;
      if (use_TTi_)
	TTi_ = new int[Tcap_][3];
      else
	TTi_ = NULL;
      S_ = new double[Vcap_][3];
    } else {
      TV_ = NULL;
      TT_ = NULL;
      VT_ = NULL;
      TTi_ = NULL;
      S_ = NULL;
    }
    X11_ = NULL;
  };

  Mesh::~Mesh()
  {
    clear();
  }

  Mesh& Mesh::clear()
  {
    Vcap_ = 0;
    Tcap_ = 0;
    nV_ = 0;
    nT_ = 0;
    use_VT_ = false;
    use_TTi_ = false;
    if (TV_) { delete[] TV_; TV_ = NULL; }
    if (TT_) { delete[] TT_; TT_ = NULL; }
    if (VT_) { delete[] VT_; VT_ = NULL; }
    if (TTi_) { delete[] TTi_; TTi_ = NULL; }
    if (S_) { delete[] S_; S_ = NULL; }
    if (X11_) { delete X11_; X11_ = NULL; }
    return *this;
  }

  Mesh& Mesh::operator=(const Mesh& M)
  {
    clear();
    type_ = M.type_;
    useVT(M.use_VT_);
    useTTi(M.use_TTi_);
    if (M.X11_) {
      X11_ = new Xtmpl(*M.X11_);
    } else {
      X11_ = NULL;
    }
    S_set(M.S_,M.nV_);
    TV_set(M.TV_,M.nT_);
    return *this;
  }

  Mesh& Mesh::check_capacity(size_t nVc, size_t nTc)
  {
    if ((nVc <= Vcap_) && (nTc <= Tcap_))
      return *this;
    while ((nVc > Vcap_) || (nTc > Tcap_)) {
      Vcap_ = Vcap_+Mesh_V_capacity_step_size;
      Tcap_ = 2*Vcap_;
    }

    int (*TV)[3] = new int[Vcap_][3];
    int (*TT)[3] = new int[Tcap_][3];
    int (*VT) = NULL;
    if (use_VT_) VT = new int[Vcap_];
    int (*TTi)[3] = NULL;
    if (use_TTi_) TTi = new int[Tcap_][3];
    double (*S)[3] = new double[Vcap_][3];

    if (TV_) {
      if (TV_) memcpy(TV,TV_,sizeof(int)*nV_*3);
      if (TT_) memcpy(TT,TT_,sizeof(int)*nT_*3);
      if (VT_) memcpy(VT,VT_,sizeof(int)*nV_);
      if (TTi_) memcpy(TTi,TTi_,sizeof(int)*nT_*3);
      if (S_) memcpy(S,S_,sizeof(double)*nV_*3);
      if (TV_) delete[] TV_;
      if (TT_) delete[] TT_;
      if (VT_) delete[] VT_;
      if (TTi_) delete[] TTi_;
      if (S_) delete[] S_;
    }

    TV_ = TV;
    TT_ = TT;
    VT_ = VT;
    TTi_ = TTi;
    S_ = S;
    return *this;
  };



  Mesh& Mesh::rebuildTT()
  {
    typedef std::pair<int,int> E_Type;
    typedef std::map<E_Type,int> ET_Type;
    int t, vi;
    int* TVt;
    E_Type E0,E1;
    ET_Type::const_iterator Ei;
    ET_Type ET;
    /* Pass 1: */
    for (t=0; t<(int)nT_; t++) {
      TVt = TV_[t];
      for (vi=0; vi<3; vi++) {
	E0 = std::pair<int,int>(TVt[(vi+1)%3],TVt[(vi+2)%3]);
	E1 = std::pair<int,int>(TVt[(vi+2)%3],TVt[(vi+1)%3]);
	Ei = ET.find(E1);
	if (Ei != ET.end()) { /* Found neighbour */
	  TT_[t][vi] = Ei->second;
	} else { /* Either on boundary, or not added yet. */
	  TT_[t][vi] = -1;
	}
	ET.insert(ET_Type::value_type(E0,t));
      }
    }
    /*
    std::cout << TTO() << std::endl;
    for (Ei=ET.begin();Ei!=ET.end();Ei++) {
      std::cout << Ei->first.first << ' '
		<< Ei->first.second << ' '
		<< Ei->second << std::endl;
    }
    */

    /* Pass 2: */
    for (t=0; t<(int)nT_; t++) {
      TVt = TV_[t];
      for (vi=0; vi<3; vi++) {
	if (TT_[t][vi]>=0) continue;
	E1 = std::pair<int,int>(TVt[(vi+2)%3],TVt[(vi+1)%3]);
	Ei = ET.find(E1);
	if (Ei != ET.end()) { /* Found neighbour */
	  TT_[t][vi] = Ei->second;
	}
      }
    }

    return *this;
  }



  Mesh& Mesh::updateVT(const int v, const int t)
  {
    if ((!use_VT_) || (v>=(int)nV_) || (t>=(int)nT_) || (VT_[v]<0))
      return *this;
    VT_[v] = t;
    return *this;
  }

  Mesh& Mesh::setVT(const int v, const int t)
  {
    if ((!use_VT_) || (v>=(int)nV_) || (t>=(int)nT_))
      return *this;
    VT_[v] = t;
    return *this;
  }

  Mesh& Mesh::updateVTtri(const int t)
  {
    int vi;
    if ((!use_VT_) || (t>=(int)nT_) || (t<0))
      return *this;
    for (vi=0; vi<3; vi++)
      updateVT(TV_[t][vi],t);
    return *this;
  }

  Mesh& Mesh::setVTtri(const int t)
  {
    int vi;
    if ((!use_VT_) || (t>=(int)nT_) || (t<0))
      return *this;
    for (vi=0; vi<3; vi++)
      setVT(TV_[t][vi],t);
    return *this;
  }

  Mesh& Mesh::updateVTtri_private(const int t0)
  {
    if (!use_VT_) return *this;
    int t, vi;
    for (t=t0; t<(int)nT_; t++)
      for (vi=0; vi<3; vi++)
	updateVT(TV_[t][vi],t);
    return *this;
  }

  Mesh& Mesh::setVTv_private(const int v0)
  {
    if (!use_VT_) return *this;
    int v;
    for (v=v0; v<(int)nV_; v++)
      setVT(v,-1);
    return *this;
  }

  Mesh& Mesh::rebuildVT()
  {
    if (!use_VT_) {
      if (VT_) {
	delete[] VT_;
	VT_ = NULL;
      }
      return *this;
    }
    if (!Vcap_)
      return *this;
    if (!VT_)
      VT_ = new int[Vcap_];
    setVTv_private(0);
    updateVTtri_private(0);
    return *this;
  }

  Mesh& Mesh::rebuildTTi()
  {
    int t, vi, v, t2, vi2;
    if (!use_TTi_) {
      if (TTi_) {
	delete[] TTi_;
	TTi_ = NULL;
      }
      return *this;
    }
    if (!Tcap_)
      return *this;
    if (!TTi_)
      TTi_ = new int[Tcap_][3];
    for (t=0; t<(int)nT_; t++) {
      for (vi=0; vi<3; vi++) {
	v = TV_[t][vi];
	t2 = TT_[t][(vi+2)%3];
	if (t2>=0) {
	  for (vi2 = 0; (vi2<3) && (TV_[t2][vi2] != v); vi2++) { }
	  if (vi2<3) {
	    TTi_[t][(vi+2)%3] = (vi2+1)%3;
	  } else {
	    /* Error! This should never happen! */
	    std::cout << "ERROR\n";
	  }
	} else {
	  TTi_[t][(vi+2)%3] = -1;
	}
      }
    }
    return *this;
  }


  Mesh& Mesh::useVT(bool use_VT)
  {
    if (use_VT_ != use_VT) {
      if ((!use_VT_) && (VT_)) {
	/* This shouldn't happen. */
	delete[] VT_;
	VT_ = NULL;
      }
      use_VT_ = use_VT;
      rebuildVT();
    }
    return *this;
  }

  Mesh& Mesh::useTTi(bool use_TTi)
  {
    if (use_TTi_ != use_TTi) {
      if ((!use_TTi_) && (TTi_)) {
	/* This shouldn't happen. */
	delete[] TTi_;
	TTi_ = NULL;
      }
      use_TTi_ = use_TTi;
      rebuildTTi();
    }
    return *this;
  }

  Mesh& Mesh::useX11(bool use_X11,
		     bool draw_text,
		     int sx, int sy,
		     double minx,
		     double maxx,
		     double miny,
		     double maxy,
		     std::string name)
  {
    if (use_X11) {
      if (!X11_) { /* Init. */
	X11_ = new Xtmpl(draw_text,sx,sy,minx,maxx,miny,maxy,name);
	redrawX11("");
      } else {
	X11_->reopen(sx,sy,draw_text);
	X11_->setAxis(minx,maxx,miny,maxy);
	redrawX11("");
      }
    } else { /* Destroy. */
      if (X11_) { /* Destroy. */
	delete X11_;
	X11_ = NULL;
      }
    }
    return *this;
  }



  Mesh& Mesh::S_set(const double (*S)[3], int nV)
  {
    nV_ = 0; /* Avoid possible unnecessary copy. */
    S_append(S,nV);
    return *this;
  }
  
  Mesh& Mesh::TV_set(const int (*TV)[3], int nT)
  {
    nT_ = 0; /* Avoid possible unnecessary copy. */
    TV_append(TV,nT);
    return *this;
  }

  Mesh& Mesh::S_append(const double (*S)[3], int nV)
  {
    check_capacity(nV_+nV,0);
    memcpy(S_+nV_,S,sizeof(double)*nV*3);
    nV_ += nV;
    if (use_VT_)
      setVTv_private(nV_-nV);
    return *this;
  }

  void Mesh::redrawX11(std::string str)
  {
    if (!X11_) return;

    int v;
    double s[3][3];
    Point s0;

    X11_->clear();
    for (int t=0;t<(int)nT_;t++) {
      s0[0] = 0.0;
      s0[1] = 0.0;
      s0[2] = 0.0;
      for (int vi=0;vi<3;vi++) {
	v = TV_[t][vi];
	Vec::copy(s[vi],S_[v]);
	Vec::accum(s0,s[vi],1.0/3.0);
      }
      if (type_==Mtype_sphere) {
	double l = std::sqrt(Vec::scalar(s0,s0));
	Vec::rescale(s0,l);
	Point r0;
	Point r1;
	Point n;
	Vec::diff(r0,s[1],s[0]);
	Vec::diff(r1,s[2],s[0]);
	Vec::cross(n,r0,r1);
	if (n[2]<0) continue;
      }
      /* Draw triangle slightly closer to center. */
      if (type_==Mtype_sphere) {
	for (int vi=0;vi<3;vi++) {
	  Vec::diff(s[vi],s[vi],s0);
	  Vec::rescale(s[vi],0.975);
	  Vec::accum(s[vi],s0);
	  Vec::rescale(s[vi],1./Vec::length(s[vi]));
	}
	X11_->arc(s[0],s[1]);
	X11_->arc(s[1],s[2]);
	X11_->arc(s[2],s[0]);
      } else {
	for (int vi=0;vi<3;vi++) {
	  Vec::diff(s[vi],s[vi],s0);
	  Vec::rescale(s[vi],0.975);
	  Vec::accum(s[vi],s0);
	}
	X11_->line(s[0],s[1]);
	X11_->line(s[1],s[2]);
	X11_->line(s[2],s[0]);
      }
      /* Draw vertex indices even closer to center. */
      for (int vi=0;vi<3;vi++) {
	  Vec::diff(s[vi],s[vi],s0);
	  Vec::rescale(s[vi],0.8);
	  Vec::accum(s[vi],s0);
      }
      for (int vi=0;vi<3;vi++) {
	std::ostringstream ss;
	ss << "(" << TV_[t][vi] << "," << TT_[t][vi] << ")";
	X11_->text(s[vi],ss.str());
      }
      /* Draw triangle indices at center. */
      {
	std::ostringstream ss;
	ss << "(" << t << ")";
	X11_->text(s0,ss.str());
      }
    }

    {
      std::string str0 = str;
      str0 += std::string(", continue");
      char* str_ = new char[str0.length()+1];
      str0.copy(str_,str0.length(),0);
      str_[str0.length()] = '\0';
      xtmpl_press_ret(str_);
      delete[] str_;
    }
  }
  
  Mesh& Mesh::TV_append(const int (*TV)[3], int nT)
  {
    check_capacity(0,nT_+nT);
    memcpy(TV_+nT_,TV,sizeof(int)*nT*3);
    nT_ += nT;
    if (use_VT_)
      updateVTtri_private(nT_-nT);
    rebuildTT();
    rebuildTTi();
    redrawX11(std::string("TV appended"));
    return *this;
  }




  /*!
   \brief Calculate the length of an edge. 

   For planes and triangular manifolds, the edge length is
   \f$L=\|s_1-s_0\|\f$.

   On the sphere, the "obvious" arccos-formula for the length of a
   geodesic may be numerically unstable for short and long edges.  Use
   the arctan-formula instead, that should handle all cases
   \f$L\in[0,\pi]\f$.
   \f{align*}{
   L &= acos((s_0 \cdot s_1)) \\
   \sin(L/2) &= \|s_1-s_0\|/2 \\
   \cos(L/2) &= \|s_0+s_1\|/2 \\
   L &= 2 \cdot atan2(\|s_1-s_0\|,\|s_0+s_1\|)
   \f}
   */
  double Mesh::edgeLength(const Dart& d) const
  {
    int t(d.t());
    if ((t<0) || (t>=(int)nT_)) return 0.0;

    int v0 = TV_[d.t()][d.vi()];
    Dart dh(d);
    dh.alpha0();
    int v1 = TV_[dh.t()][dh.vi()];
    const double* s0 = S_[v0];
    const double* s1 = S_[v1];
    Point e;
    Vec::diff(e,s1,s0);
    double len = Vec::length(e);

    if (type_==Mesh::Mtype_sphere) {
      Point ssum;
      Vec::sum(ssum,s1,s0);
      len = 2.0*std::atan2(len,Vec::length(ssum));
    }

    return len;
  }

  /*!
    \brief Calculate a triangle area.

    Notation:\n
    \f$s_k\f$ are the cartesian coordinates of vertex \f$k\f$.\n
    \f$e_0=s_2-s_1\f$ is the CCW edge vector opposite node 0,
    and similarly for the other nodes.\n
    \f$n_0=e_2\times(-e_1)=e_1\times e_2\f$ is an unnormalised
    outwards triangle normal vector.

    For planes, calculate the signed area, from the z-component of
    \f$(n_0+n_1+n_2)/6\f$.\n
    For manifolds, calculate the absolute area, from
    \f$A=\|n_0+n_1+n_3\|/6\f$.\n
    For spheres, calculate the CCW interior geodesic triangle area,
    with range \f$[0,4\pi)\f$.

    The spherical geodesic triangle area is given by the spherical
    excess formula \f$A = \theta_0+\theta_1+\theta_2-\pi\f$, where
    \f$\theta_i\f$ is the interior angle at node i.
   \f{align*}{
   d_1 &= e_1-s_0(s_0\cdot e_1) \\
   d_2 &= e_2-s_0(s_0\cdot e_2) \\
   \|d_1\| \|d_2\| \sin \theta_0 &= s_0\cdot(d_1\times d_2) \\
   \|d_1\| \|d_2\| \cos \theta_0 &= - (d_1 \cdot d_2) \\
   \theta_0 &= \mbox{atan2}(s_0\cdot(d_1\times d_2),- (d_1 \cdot d_2))
               \mod [0,2\pi)
   \f}
   Rewrite in terms of the original edge vectors:
   \f{align*}{
   s_0\cdot(d_1\times d_2) &= s_0\cdot(e_1\times e_2
                             - (e_1\times s_0)(s_0\cdot e_2)
                             - (e_2\times s_0)(s_0\cdot e_1))
                       = s_0\cdot(e_1\times e_2) \\
   (d_1\cdot d_2) &= (e_1\cdot e_2) - (s_0\cdot e_1)(s_0\cdot e_2) \\
   \theta_0 &= atan2(s_0\cdot(e_1\times e_2),
                      (s_0\cdot e_1)(s_0\cdot e_2) - (e_1 \cdot e_2))
	       \mod [0,2\pi)
   \f}
   The two remaining angles, \f$\theta_1\f$ and \f$\theta_2\f$, are
   obtained by permuting indices.

   If two of the vertices are antipodes, say \f$s_1=-s_0\f$, the
   formula breaks down, with both arguments to \p atan2 equal to zero.
   Such triangles are not uniquely defined, since the geodesic between
   \f$s_0\f$ and \f$-s_0\f$ is not unique.

   If \f$s_0\cdot(e_1\times e_2)\f$ is negative, the triangle covers
   more than a hemisphere, and is non-convex (its complement is
   convex), with area \f$>2\pi\f$.

  */
  /*
    Don't use the following, since they rely on the edge lengths to be known.

    Heron's formula:
    a,b,c edge lengths
    s = (a+b+c)/2
    Area = sqrt(s(s-a)(s-b)(s-c)) 
    
    Numerically stable version from
    http://www.eecs.berkeley.edu/~wkahan/Triangle.pdf
    a >= b >= c
    Area = sqrt( (a+(b+c)) (c-(a-b)) (c+(a-b)) (a+(b-c)) )/4

    l'Huilier's Theorem for spherical triangle areas:
    a,b,c edge lengths
    s = (a+b+c)/2
    tan(E / 4) = sqrt(tan(s / 2) 
                      tan((s - a) / 2)
		      tan((s - b) / 2)
		      tan((s - c) / 2))
    Area = E  (E = spherical excess)
  */
  double Mesh::triangleArea(int t) const
  {
    if ((t<0) || (t>=(int)nT_)) return 0.0;

    Dart dh(Dart(*this,t));
    int v0 = TV_[dh.t()][dh.vi()];
    dh.orbit2();
    int v1 = TV_[dh.t()][dh.vi()];
    dh.orbit2();
    int v2 = TV_[dh.t()][dh.vi()];
    const double* s0 = S_[v0];
    const double* s1 = S_[v1];
    const double* s2 = S_[v2];
    Point e0, e1, e2;
    Vec::diff(e0,s2,s1);
    Vec::diff(e1,s0,s2);
    Vec::diff(e2,s1,s0);

    double area;

    switch (type_) {
    case Mesh::Mtype_manifold:
      {
	/* Calculate the upwards unscaled normal(s), calculate length. */
	Point n0, n1, n2;
	Vec::cross(n0,e1,e2);
	Vec::cross(n1,e2,e0);
	Vec::cross(n2,e0,e1);
	Vec::accum(n0,n1);
	Vec::accum(n0,n2);
	area = Vec::length(n0)/6.0;
      }
      break;
    case Mesh::Mtype_plane:
      {
	/* Calculate the upwards unscaled normal(s), extract z-component. */
	area = (Vec::cross2(e1,e2)+
		Vec::cross2(e2,e0)+
		Vec::cross2(e0,e1))/6.0;
      }
      break;
    case Mesh::Mtype_sphere:
      {
	/* Calculate the spherical excess. */
	Point n0, n1, n2;
	Vec::cross(n0,e1,e2);
	Vec::cross(n1,e2,e0);
	Vec::cross(n2,e0,e1);
	/* Note: std::fmod calculates signed modulus, so use other trick. */
	int nneg = 0;
	double theta0 = std::atan2(Vec::scalar(s0,n0),
				   Vec::scalar(s0,e1)
				   *Vec::scalar(s0,e2)
				   -Vec::scalar(e1,e2));
	if (theta0<0) nneg++;
	double theta1 = std::atan2(Vec::scalar(s1,n1),
				   Vec::scalar(s1,e2)
				   *Vec::scalar(s1,e0)
				   -Vec::scalar(e2,e0));
	if (theta1<0) nneg++;
	double theta2 = std::atan2(Vec::scalar(s2,n2),
				   Vec::scalar(s2,e0)
				   *Vec::scalar(s2,e1)
				   -Vec::scalar(e0,e1));
	if (theta2<0) nneg++;
	area = theta0+theta1+theta2+static_cast<double>(2*nneg-1)*M_PI;
	/*
	  // L'Huilier code, don't use:
	  Dart dh(*this,t);
	  double a(edgeLength(dh));
	  dh.orbit2();
	  double b(edgeLength(dh));
	  dh.orbit2();
	  double c(edgeLength(dh));
	  double s((a+b+c)/2.0);
	  double tanE4(std::sqrt(std::tan(s/2.0)*
	                         std::tan((s-a)/2.0)*
				 std::tan((s-b)/2.0)*
				 std::tan((s-c)/2.0)));
          area = 4.0*std::atan(tanE4);
	*/
      }
      break;
    default:
      /* ERROR: This should never be reached. */
      NOT_IMPLEMENTED;
      area = 0.0;
    }

    return area;
  }

  /*!
    Calculate triangle circumcenter

    For planes, we use a linear combination of the vertices, with
    weights obtained from
    http://wapedia.mobi/en/Circumscribed_circle#2. \n
    Rewriting in our notation, the weights and circumcenter are given by
    \f{align*}{
    a_0  &= - \frac{\|e_0\|^2 (e_1\cdot e_2)}{2\|e_1\times e_2\|^2}
    = \frac{\|e_0\|^2}{4 A \tan(\theta_0)} \\
    c &= a_0s_0+a_1s_1+a_2s_2
    \f}
    where formulas for \f$a_1\f$ and \f$a_2\f$ are given by index
    permutation.
    
    On the sphere, the normalised flat triangle normal is the circumcenter.
    
    \see Mesh::triangleArea
    \see Mesh::triangleCircumcircleRadius
  */
  void Mesh::triangleCircumcenter(int t, double* c) const
  {
    if ((t<0) || (t>=(int)nT_)) {
      c[0] = 0.0;
      c[1] = 0.0;
      c[2] = 0.0;
      return;
    }

    int v0 = TV_[t][0];
    int v1 = TV_[t][1];
    int v2 = TV_[t][2];
    const double* s0 = S_[v0];
    const double* s1 = S_[v1];
    const double* s2 = S_[v2];
    Point e0, e1, e2;
    Vec::diff(e0,s2,s1);
    Vec::diff(e1,s0,s2);
    Vec::diff(e2,s1,s0);

    switch (type_) {
    case Mesh::Mtype_manifold:
      /* TODO: Implement? Need more manifold theory for that! */
      NOT_IMPLEMENTED;
      {
	/* Calculate centroid instead of circumcenter. */
	Vec::copy(c,s0);
	Vec::rescale(c,1.0/3.0);
	Vec::accum(c,s1,1/3.0);
	Vec::accum(c,s2,1/3.0);
      }
      break;
    case Mesh::Mtype_plane:
      {
	Point n0, n1, n2;
	Vec::cross(n0,e1,e2);
	Vec::cross(n1,e2,e0);
	Vec::cross(n2,e0,e1);
	Vec::accum(n0,n1);
	Vec::accum(n0,n2);
	double scale(-4.5/Vec::scalar(n0,n0));
	Vec::scale(c,s0,scale*Vec::scalar(e0,e0)*Vec::scalar(e1,e2));
	Vec::accum(c,s1,scale*Vec::scalar(e1,e1)*Vec::scalar(e2,e0));
	Vec::accum(c,s2,scale*Vec::scalar(e2,e2)*Vec::scalar(e0,e1));
      }
      break;
    case Mesh::Mtype_sphere:
      {
	/* The triangle normal is equal to the circumcenter. */
	Point tmp;
	Vec::cross(c,e1,e2);
	Vec::cross(tmp,e2,e0);
	Vec::accum(c,tmp);
	Vec::cross(tmp,e0,e1);
	Vec::accum(c,tmp);
	Vec::rescale(c,1.0/Vec::length(c));
      }
      break;
    }

    return;
  }

  /*!
    \brief Calculate the radius of the triangle circumcircle

    We use the formula given at
    http://wapedia.mobi/en/Circumscribed_circle#2. \n
    Rewriting in our notation, the radius of the circumcircle
    is given by
    \f{align*}{
    r  &= \frac{3\|e_0\| \|e_1\| \|e_2\|}{2\|n_0+n_1+n_2\|}
   \f}

    \see Mesh::triangleArea
    \see Mesh::triangleCircumcenter
   */
  double Mesh::triangleCircumcircleRadius(int t) const
  {
    if ((t<0) || (t>=(int)nT_)) return -1.0;

    int v0 = TV_[t][0];
    int v1 = TV_[t][1];
    int v2 = TV_[t][2];
    const double* s0 = S_[v0];
    const double* s1 = S_[v1];
    const double* s2 = S_[v2];
    Point e0, e1, e2;
    Vec::diff(e0,s2,s1);
    Vec::diff(e1,s0,s2);
    Vec::diff(e2,s1,s0);

    Point n0, n1, n2;
    Vec::cross(n0,e1,e2);
    Vec::cross(n1,e2,e0);
    Vec::cross(n2,e0,e1);
    Vec::accum(n0,n1);
    Vec::accum(n0,n2);

    return ((3.0
	     *Vec::length(e0)*Vec::length(e1)
	     *Vec::length(e2))
	    /(2.0*Vec::length(n0)));
  }

  double Mesh::triangleShortestEdge(int t) const
  {
    if ((t<0) || (t>=(int)nT_)) return 0.0;

    double len;
    Dart dh(*this,t);
    double len_min = edgeLength(dh);
    dh.orbit2();
    len = edgeLength(dh);
    if (len < len_min)
      len_min = len;
    dh.orbit2();
    len = edgeLength(dh);
    if (len < len_min)
      len_min = len;

    return len_min;
  }

  double Mesh::triangleLongestEdge(int t) const
  {
    if ((t<0) || (t>=(int)nT_)) return 0.0;

    double len;
    Dart dh(*this,t);
    double len_max = edgeLength(dh);
    dh.orbit2();
    len = edgeLength(dh);
    if (len > len_max)
      len_max = len;
    dh.orbit2();
    len = edgeLength(dh);
    if (len > len_max)
      len_max = len;

    return len_max;
  }


  double Mesh::edgeEncroached(const Dart& d, const double s[3]) const
  /* > --> encroached */
  {
    int t(d.t());
    if ((t<0) || (t>=(int)nT_)) return -1.0;

    Dart dh(d);
    int v0 = TV_[t][dh.vi()];
    dh.orbit2();
    int v1 = TV_[t][dh.vi()];

    /* Construct a mirror of s reflected in v0-->v1 */
    double sm[3]; 
    switch (type_) {
    case Mesh::Mtype_manifold:
      //	return predicates::orient3d(M_->S[]);
    /* TODO: Implement. */
      NOT_IMPLEMENTED;
      sm[0] = 0.0;
      sm[1] = 0.0;
      sm[2] = 0.0;
      break;
    case Mesh::Mtype_plane:
      /* e = s1-s0
         sv = s-s0
         sm = s0 + (2*e*e'*sv/elen2 - sv)
            = s0 + (2*e*se - sv)
       */
      {
	double s0[3];
	s0[0] = S_[v0][0];
	s0[1] = S_[v0][1];
	s0[2] = S_[v0][2];
	double e[3];
	e[0] = S_[v1][0]-s0[0];
	e[1] = S_[v1][1]-s0[1];
	e[2] = S_[v1][2]-s0[2];
	double elen2 = e[0]*e[0]+e[1]*e[1]+e[2]*e[2];
	double sv[3];
	sv[0] = s[0]-s0[0];
	sv[1] = s[1]-s0[1];
	sv[2] = s[2]-s0[2];
	double se = (sv[0]*e[0]+sv[1]*e[1]+sv[2]*e[2])/elen2;
	sm[0] = s0[0] + (2*se*e[0] - sv[0]);
	sm[1] = s0[1] + (2*se*e[1] - sv[1]);
	sm[2] = s0[2] + (2*se*e[2] - sv[2]);
      }
      break;
    case Mesh::Mtype_sphere:
      /* TODO: Implement. */
      NOT_IMPLEMENTED;
      /*      Point zero = {0.,0.,0.}; */
      sm[0] = 0.0;
      sm[1] = 0.0;
      sm[2] = 0.0;
      break;
    }
    
    switch (type_) {
    case Mesh::Mtype_manifold:
      //	return predicates::orient3d(M_->S[]);
      /* TODO: Implement. */
      NOT_IMPLEMENTED;
      break;
    case Mesh::Mtype_plane:
      return predicates::incircle(S_[v0],S_[v1],s,sm);
      break;
    case Mesh::Mtype_sphere:
      //      return -predicates::orient3d(S_[v0],S_[v1],S_[v2],s);
      /* TODO: Implement. */
      NOT_IMPLEMENTED;
      break;
    }
    /* This should never be reached. */
    return 0.0;
  }





  double Mesh::inLeftHalfspace(const Dart& d, const double s[3]) const
  {
    Dart dh(d);
    int v0, v1;
    if (d.isnull()) return 0.0; /* TODO: should show a warning somewhere... */
    const int* tp = TV_[dh.t()];
    v0 = tp[dh.vi()];
    dh.orbit2();
    v1 = tp[dh.vi()];
    switch (type_) {
    case Mesh::Mtype_manifold:
      //	return predicates::orient3d(M_->S[]);
      NOT_IMPLEMENTED;
      break;
    case Mesh::Mtype_plane:
      return predicates::orient2d(S_[v0],S_[v1],s);
      break;
    case Mesh::Mtype_sphere:
      Point zero = {0.,0.,0.};
      return -predicates::orient3d(S_[v0],S_[v1],zero,s);
      break;
    }
    /* This should never be reached. */
    return 0.0;
  }

  double Mesh::inCircumcircle(const Dart& d, const double s[3]) const
  {
    Dart dh(d);
    int v0, v1, v2;
    if (d.isnull()) return 0.0; /* TODO: should show a warning somewhere... */
    const int* tp = TV_[dh.t()];
    v0 = tp[dh.vi()];
    dh.orbit2();
    v1 = tp[dh.vi()];
    dh.orbit2();
    v2 = tp[dh.vi()];
    switch (type_) {
    case Mesh::Mtype_manifold:
      //	return predicates::orient3d(M_->S[]);
      break;
    case Mesh::Mtype_plane:
      return predicates::incircle(S_[v0],S_[v1],S_[v2],s);
      break;
    case Mesh::Mtype_sphere:
      return -predicates::orient3d(S_[v0],S_[v1],S_[v2],s);
      break;
    }
    /* This should never be reached. */
    return 0.0;
  }

  bool Mesh::circumcircleOK(const Dart& d) const
  {
    Dart dh(d);
    int v;
    double result;
    if (d.isnull()) return true; /* TODO: should show a warning somewhere... */
    if (d.onBoundary()) return true; /* Locally optimal, OK. */
    dh.orbit0rev().orbit2();
    v = TV_[dh.t()][dh.vi()];
    result = inCircumcircle(d,S_[v]);
    std::cout << "Dart=" << d
	      << " Node=" << v
	      << std::scientific << " result=" << result
	      << std::endl;
    if  (result > MESH_EPSILON)
      return false;
    /* For robusness, check with the reverse dart as well: */
    dh = d;
    dh.orbit2rev();
    v = TV_[dh.t()][dh.vi()];
    dh.orbit2();
    dh.orbit1();
    result = inCircumcircle(dh,S_[v]);
    std::cout << "Dart=" << dh
	      << " Node=" << v
	      << std::scientific << " result=" << result
	      << std::endl;
    if  (result > MESH_EPSILON)
      return false;
    return true;
  }


  /*! \brief Swap an edge

     \verbatim
       2         2
      /0\       /|\
     0d--1 --> 00|11
      \1/       \d/
       3         3
     \endverbatim
     Dart 0-1 --> 3-2
    
  */
  Dart Mesh::swapEdge(const Dart& d)
  {
    Dart dh(d);
    int vi;
    int v_list[4];
    int t0, t1;
    int tt_list[4];
    int tti_list[4];
    if (d.edir()<0) dh.alpha1(); /* Correct dart orientation */

    /* Step 1: Store geometry information. */
    t0 = dh.t();
    vi = dh.vi();
    v_list[0] = TV_[t0][vi];
    tt_list[0] = TT_[t0][vi];
    if (use_TTi_) tti_list[0] = TTi_[t0][vi];
    dh.orbit2();
    vi = dh.vi();
    v_list[1] = TV_[t0][vi];
    tt_list[1] = TT_[t0][vi];
    if (use_TTi_) tti_list[1] = TTi_[t0][vi];
    dh.orbit2();
    v_list[2] = TV_[t0][dh.vi()];
    dh.orbit2rev().orbit0();
    t1 = dh.t();
    if (t0 == t1) { dh = d; return dh; } /* ERROR: Boundary edge */
    vi = dh.vi();
    tt_list[2] = TT_[t1][vi];
    if (use_TTi_) tti_list[2] = TTi_[t1][vi];
    dh.orbit2();
    vi = dh.vi();
    tt_list[3] = TT_[t1][vi];
    if (use_TTi_) tti_list[3] = TTi_[t1][vi];
    dh.orbit2();
    v_list[3] = TV_[t1][dh.vi()];

    /* Step 2: Overwrite with new triangles. */
    TV_[t0][0] = v_list[0];
    TV_[t0][1] = v_list[3];
    TV_[t0][2] = v_list[2];
    TT_[t0][0] = t1;
    TT_[t0][1] = tt_list[1];
    TT_[t0][2] = tt_list[2];
    if (use_TTi_) {
      TTi_[t0][0] = 0;
      TTi_[t0][1] = tti_list[1];
      TTi_[t0][2] = tti_list[2];
    }
    TV_[t1][0] = v_list[1];
    TV_[t1][1] = v_list[2];
    TV_[t1][2] = v_list[3];
    TT_[t1][0] = t0;
    TT_[t1][1] = tt_list[3];
    TT_[t1][2] = tt_list[0];
    if (use_TTi_) {
      TTi_[t1][0] = 0;
      TTi_[t1][1] = tti_list[3];
      TTi_[t1][2] = tti_list[0];
    }

    /* Step 3: Relink neighbouring triangles. */
    if (use_TTi_) {
      if (TT_[t0][1]>=0) TTi_[TT_[t0][1]][TTi_[t0][1]] = 1;
      if (TT_[t0][2]>=0) TTi_[TT_[t0][2]][TTi_[t0][2]] = 2;
      if (TT_[t1][1]>=0) TTi_[TT_[t1][1]][TTi_[t1][1]] = 1;
      if (TT_[t1][2]>=0) TTi_[TT_[t1][2]][TTi_[t1][2]] = 2;
      if (TT_[t0][1]>=0) TT_[TT_[t0][1]][TTi_[t0][1]] = t0;
      if (TT_[t0][2]>=0) TT_[TT_[t0][2]][TTi_[t0][2]] = t0;
      if (TT_[t1][1]>=0) TT_[TT_[t1][1]][TTi_[t1][1]] = t1;
      if (TT_[t1][2]>=0) TT_[TT_[t1][2]][TTi_[t1][2]] = t1;
    } else {
      if (TT_[t0][1]>=0) {
	dh = Dart(*this,t0,1,2).orbit0rev();
	dh.orbit2();
	TT_[dh.t()][dh.vi()] = t0;
      }
      if (TT_[t0][2]>=0) {
	dh = Dart(*this,t0,1,0).orbit0rev();
	dh.orbit2();
	TT_[dh.t()][dh.vi()] = t0;
      }
      if (TT_[t1][1]>=0) {
	dh = Dart(*this,t1,1,2).orbit0rev();
	dh.orbit2();
	TT_[dh.t()][dh.vi()] = t1;
      }
      if (TT_[t1][2]>=0) {
	dh = Dart(*this,t1,1,0).orbit0rev();
	dh.orbit2();
	TT_[dh.t()][dh.vi()] = t1;
      }
    }

    /* Link vertices to triangles */
    if (use_VT_) {
      setVTtri(t1);
      setVTtri(t0);
    }

    /* Debug code: */
    /* 
    std::cout << "TT is \n" << TTO();
    rebuildTT();
    std::cout << "TT should be \n" << TTO();
    if (use_TTi_) {
      std::cout << "TTi is \n" << TTiO();
      rebuildTTi();
      std::cout << "TTi should be \n" << TTiO();
    }
    */

    redrawX11("Edge swapped");
    
    return Dart(*this,t0,1,1);
  }
  
  /*!
     \verbatim
     2           2
    /|\         /|\
   / | \       /1d2\
  1 0|1 3 --> 1--v--3
   \ | /       \0|3/
    \d/         \|/
     0           0
     \endverbatim
   
     Dart 0-2 --> v-2
  */
  Dart Mesh::splitEdge(const Dart& d, int v)
  {
    Dart dh(d);
    int t, vi;
    int v0, v1, v2, v3;
    int t0, t1, t2, t3;
    int tt_list[4];
    int tti_list[4];
    if (d.edir()<0) dh.alpha0(); /* Correct dart orientation */

    /* Step 1: Store geometry information. */
    /* Go through t0: */
    t0 = dh.t();
    vi = dh.vi();
    v0 = TV_[t0][vi];
    tt_list[1] = TT_[t0][vi];
    if (use_TTi_) tti_list[1] = TTi_[t0][vi];
    dh.orbit2();
    vi = dh.vi();
    v2 = TV_[t0][vi];
    tt_list[0] = TT_[t0][vi];
    if (use_TTi_) tti_list[0] = TTi_[t0][vi];
    dh.orbit2();
    vi = dh.vi();
    v1 = TV_[t0][vi];
    dh.orbit2();

    bool on_boundary = dh.onBoundary();
    if (!on_boundary) {
      /* Go through t1: */
      dh.orbit1();
      t1 = dh.t();
      vi = dh.vi();
      tt_list[3] = TT_[t1][vi];
      if (use_TTi_) tti_list[3] = TTi_[t1][vi];
      dh.orbit2();
      vi = dh.vi();
      tt_list[2] = TT_[t1][vi];
      if (use_TTi_) tti_list[2] = TTi_[t1][vi];
      dh.orbit2();
      vi = dh.vi();
      v3 = TV_[t1][vi];
    } else {
      v3 = -1;
      tt_list[2] = -1;
      tt_list[3] = -1;
      if (use_TTi_) {
	tti_list[2] = -1;
	tti_list[3] = -1;
      }
    }

    /* Step 2: Overwrite one/two triangles, create two/four new. */
    /* t0 = t0; */
    if (on_boundary) {
      t1 = nT_;
      check_capacity(0,nT_+1);
      t2 = -1;
      t3 = -1;
    } else {
      /* t1 = t1; */
      t2 = nT_;
      t3 = nT_+1;
      check_capacity(0,nT_+2);
    }
    /* t0 */
    t = t0;
    TV_[t][0] = v;
    TV_[t][1] = v1;
    TV_[t][2] = v0;
    TT_[t][0] = tt_list[0];
    TT_[t][1] = t3;
    TT_[t][2] = t1;
    if (use_TTi_) {
      TTi_[t][0] = tti_list[0];
      TTi_[t][1] = 2;
      TTi_[t][2] = 1;
    }
    /* t1 */
    t = t1;
    TV_[t][0] = v;
    TV_[t][1] = v2;
    TV_[t][2] = v1;
    TT_[t][0] = tt_list[1];
    TT_[t][1] = t0;
    TT_[t][2] = t2;
    if (use_TTi_) {
      TTi_[t][0] = tti_list[1];
      TTi_[t][1] = 2;
      TTi_[t][2] = 1;
    }
    if (!on_boundary) {
      /* t2 */
      t = t2;
      TV_[t][0] = v;
      TV_[t][1] = v3;
      TV_[t][2] = v2;
      TT_[t][0] = tt_list[2];
      TT_[t][1] = t1;
      TT_[t][2] = t3;
      if (use_TTi_) {
	TTi_[t][0] = tti_list[2];
	TTi_[t][1] = 2;
	TTi_[t][2] = 1;
      }
      /* t3 */
      t = t3;
      TV_[t][0] = v;
      TV_[t][1] = v0;
      TV_[t][2] = v3;
      TT_[t][0] = tt_list[3];
      TT_[t][1] = t2;
      TT_[t][2] = t0;
      if (use_TTi_) {
	TTi_[t][0] = tti_list[3];
	TTi_[t][1] = 2;
	TTi_[t][2] = 1;
      }
    }

    /* Step 3: Relink neighbouring triangles. */
    if (use_TTi_) {
      if (TT_[t0][0]>=0) TTi_[TT_[t0][0]][TTi_[t0][0]] = 0;
      if (TT_[t1][0]>=0) TTi_[TT_[t1][0]][TTi_[t1][0]] = 0;
      if (TT_[t0][0]>=0) TT_[TT_[t0][0]][TTi_[t0][0]] = t0;
      if (TT_[t1][0]>=0) TT_[TT_[t1][0]][TTi_[t1][0]] = t1;
      if (!on_boundary) {
	if (TT_[t2][0]>=0) TTi_[TT_[t2][0]][TTi_[t2][0]] = 0;
	if (TT_[t3][0]>=0) TTi_[TT_[t3][0]][TTi_[t3][0]] = 0;
	if (TT_[t2][0]>=0) TT_[TT_[t2][0]][TTi_[t2][0]] = t2;
	if (TT_[t3][0]>=0) TT_[TT_[t3][0]][TTi_[t3][0]] = t3;
      }
    } else {
      if (TT_[t0][0]>=0) {
	dh = Dart(*this,t0,1,1).orbit0rev();
	dh.orbit2();
	TT_[dh.t()][dh.vi()] = t0;
      }
      if (TT_[t1][0]>=0) {
	dh = Dart(*this,t1,1,1).orbit0rev();
	dh.orbit2();
	TT_[dh.t()][dh.vi()] = t1;
      }
      if (!on_boundary) {
	if (TT_[t2][0]>=0) {
	  dh = Dart(*this,t2,1,1).orbit0rev();
	  dh.orbit2();
	  TT_[dh.t()][dh.vi()] = t2;
	}
	if (TT_[t3][0]>=0) {
	  dh = Dart(*this,t3,1,1).orbit0rev();
	  dh.orbit2();
	  TT_[dh.t()][dh.vi()] = t3;
	}
      }
    }

    /* Step 4: Update triangle count. */
    if (on_boundary)
      nT_ = nT_+1;
    else
      nT_ = nT_+2;
  
    /* Link vertices to triangles */
    if (use_VT_) {
      if (!on_boundary) {
	setVTtri(t3);
	setVTtri(t2);
      }
      setVTtri(t1);
      setVTtri(t0);
    }

    /* Debug code: */
    /*
    std::cout << "TT is \n" << TTO();
    rebuildTT();
    std::cout << "TT should be \n" << TTO();
    if (use_TTi_) {
      std::cout << "TTi is \n" << TTiO();
      rebuildTTi();
      std::cout << "TTi should be \n" << TTiO();
    }
    */

    redrawX11("Edge split");
    
    return Dart(*this,t1,1,0);
  }

  /*!
     \verbatim
      2          2
     |  \       |\ \
     |   \      |2\1\
     |    1 --> | v--1
     |   /      | d0/
     | d/       |/ /
      0          0
     \endverbatim
   
     Dart 0-1 --> v-1
  */
  Dart Mesh::splitTriangle(const Dart& d, int v)
  {
    Dart dh(d);
    int t, vi;
    int v0, v1, v2;
    int t0, t1, t2;
    int tt_list[3];
    int tti_list[3];
    if (d.edir()<0) dh.alpha1(); /* Correct dart orientation */

    /* Step 1: Store geometry information. */
    t = dh.t();
    vi = dh.vi();
    v0 = TV_[t][vi];
    tt_list[1] = TT_[t][vi];
    if (use_TTi_) tti_list[1] = TTi_[t][vi];
    dh.orbit2();
    t = dh.t();
    vi = dh.vi();
    v1 = TV_[t][vi];
    tt_list[2] = TT_[t][vi];
    if (use_TTi_) tti_list[2] = TTi_[t][vi];
    dh.orbit2();
    t = dh.t();
    vi = dh.vi();
    v2 = TV_[t][vi];
    tt_list[0] = TT_[t][vi];
    if (use_TTi_) tti_list[0] = TTi_[t][vi];
    dh.orbit2();

    /* Step 2: Overwrite one triangle, create two new. */
    t0 = t;
    t1 = nT_;
    t2 = nT_+1;
    check_capacity(0,nT_+2);
    TV_[t0][0] = v;
    TV_[t0][1] = v0;
    TV_[t0][2] = v1;
    TT_[t0][0] = tt_list[0];
    TT_[t0][1] = t1;
    TT_[t0][2] = t2;
    if (use_TTi_) {
      TTi_[t0][0] = tti_list[0];
      TTi_[t0][1] = 2;
      TTi_[t0][2] = 1;
    }
    TV_[t1][0] = v;
    TV_[t1][1] = v1;
    TV_[t1][2] = v2;
    TT_[t1][0] = tt_list[1];
    TT_[t1][1] = t2;
    TT_[t1][2] = t0;
    if (use_TTi_) {
      TTi_[t1][0] = tti_list[1];
      TTi_[t1][1] = 2;
      TTi_[t1][2] = 1;
    }
    TV_[t2][0] = v;
    TV_[t2][1] = v2;
    TV_[t2][2] = v0;
    TT_[t2][0] = tt_list[2];
    TT_[t2][1] = t0;
    TT_[t2][2] = t1;
    if (use_TTi_) {
      TTi_[t2][0] = tti_list[2];
      TTi_[t2][1] = 2;
      TTi_[t2][2] = 1;
    }

    /* Step 3: Relink neighbouring triangles. */
    if (use_TTi_) {
      if (TT_[t0][0]>=0) TTi_[TT_[t0][0]][TTi_[t0][0]] = 0;
      if (TT_[t1][0]>=0) TTi_[TT_[t1][0]][TTi_[t1][0]] = 0;
      if (TT_[t2][0]>=0) TTi_[TT_[t2][0]][TTi_[t2][0]] = 0;
      if (TT_[t0][0]>=0) TT_[TT_[t0][0]][TTi_[t0][0]] = t0;
      if (TT_[t1][0]>=0) TT_[TT_[t1][0]][TTi_[t1][0]] = t1;
      if (TT_[t2][0]>=0) TT_[TT_[t2][0]][TTi_[t2][0]] = t2;
    } else {
      if (TT_[t0][0]>=0) {
	dh = Dart(*this,t0,1,1).orbit0rev();
	dh.orbit2();
	TT_[dh.t()][dh.vi()] = t0;
      }
      if (TT_[t1][0]>=0) {
	dh = Dart(*this,t1,1,1).orbit0rev();
	dh.orbit2();
	TT_[dh.t()][dh.vi()] = t1;
      }
      if (TT_[t2][0]>=0) {
	dh = Dart(*this,t2,1,1).orbit0rev();
	dh.orbit2();
	TT_[dh.t()][dh.vi()] = t2;
      }
    }

    /* Step 4: Update triangle count. */
    nT_ = nT_+2;

    /* Link vertices to triangles */
    if (use_VT_) {
      setVTtri(t2);
      setVTtri(t1);
      setVTtri(t0);
    }

    /* Debug code: */
    /*
    std::cout << "TT is \n" << TTO();
    rebuildTT();
    std::cout << "TT should be \n" << TTO();
    if (use_TTi_) {
      std::cout << "TTi is \n" << TTiO();
      rebuildTTi();
      std::cout << "TTi should be \n" << TTiO();
    }
    */
    
    redrawX11("Triangle split");

    return Dart(*this,t0,1,0);
  }



  /*!
    Alg 9.1

    If the point is located within the triangulation domain,
    delta_min and the returned Dart correspond to the triangle edge
    with smallest distance, as measured by inLeftHalfspace.

    If the point is not found, a null Dart is returned.
   */
  Dart Mesh::locatePoint(const Dart& d0,
			 const Point s,
			 double* delta_min) const
  {
    Dart dart;
    if (d0.isnull())
      dart = Dart(*this,0);
    else
      dart = Dart(*this,d0.t(),1,d0.vi());
    Dart dart_start(dart);
    double delta;
    Dart dart_min = Dart();
    while (1) {
      std::cout << dart_start << ' '
		<< dart << ' '
		<< inLeftHalfspace(dart,s)
		<< std::endl;
      delta = inLeftHalfspace(dart,s);
      if (dart_min.isnull() || (delta<*delta_min)) {
	dart_min = dart;
	*delta_min = delta;
      }
      if (delta >= -MESH_EPSILON) {
	dart.orbit2();
	if (dart==dart_start)
	  return dart_min;
      } else {
	if (dart.onBoundary())
	  return Dart();
	dart.alpha2();
	dart_start = dart;
	dart_start.alpha0();
	dart.alpha1();
	dart_min = dart_start;
	*delta_min = -delta;
      }
    }

    return Dart();
  }

  /*!
    Alg 9.1 modified to locate a pre-existing vertex.

    If the vertex is not found, a null Dart is returned.
   */
  Dart Mesh::locateVertex(const Dart& d0,
			  const int v) const
  {
    if (use_VT_) {
      int t;
      t = VT_[v];
      if (t<0) /* Vertex not connected to any triangles. */
	return Dart();
      if (TV_[t][0] == v)
	return Dart(*this,t,1,0);
      if (TV_[t][1] == v)
	return Dart(*this,t,1,1);
      if (TV_[t][2] == v)
	return Dart(*this,t,1,2);
      return Dart(); /* ERROR: Inconsistent data structures! */
    }

    int i;
    Dart dart;
    if (d0.isnull())
      dart = Dart(*this,0);
    else
      dart = Dart(*this,d0.t(),1,d0.vi());
    Dart dart_start(dart);
    double delta;
    Dart dart_min = Dart();
    double* s = &(S_[v][0]);
    double delta_min = 0.0;
    while (1) {
      std::cout << dart_start << ' '
		<< dart << ' '
		<< inLeftHalfspace(dart,s)
		<< std::endl;
      for (i=0;i<3;i++) {
	if (TV_[dart.t()][dart.vi()] == v)
	  return dart;
	dart.orbit2();
      }

      delta = inLeftHalfspace(dart,s);
      if (dart_min.isnull() || (delta<delta_min)) {
	dart_min = dart;
	delta_min = delta;
      }
      if (delta >= -MESH_EPSILON) {
	dart.orbit2();
	if (dart==dart_start) {
	  for (i=0;i<3;i++) {
	    if (TV_[dart.t()][dart.vi()] == v)
	      return dart;
	    dart.orbit2();
	  }
	  return Dart(); /* ERROR: Point located, but not the vertex itself. */
	}
      } else {
	if (dart.onBoundary())
	  return Dart();
	dart.alpha2();
	dart_start = dart;
	dart_start.alpha0();
	dart.alpha1();
	dart_min = dart_start;
	delta_min = -delta;
      }
    }

    return Dart();
  }




  MOAint3 Mesh::TVO() const { return MOAint3(TV_,nT_); };
  MOAint3 Mesh::TTO() const { return MOAint3(TT_,nT_); };
  MOAint Mesh::VTO() const { return MOAint(VT_,nV_); };
  MOAint3 Mesh::TTiO() const { return MOAint3(TTi_,nT_); };
  MOAdouble3 Mesh::SO() const { return MOAdouble3(S_,nV_); };







  Dart& Dart::alpha0()
  {
    vi_ = (vi_ + (3+edir_) ) % 3;
    edir_ = -edir_;
    return *this;
  }

  Dart& Dart::alpha1()
  {
    edir_ = -edir_;
    return *this;
  }

  Dart& Dart::alpha2()
  {
    if (!M_->use_TTi_) {
      int vi;
      int v = M_->TV_[t_][vi_];
      int t = M_->TT_[t_][(vi_+(3-edir_))%3];
      if (t<0) return *this;
      for (vi = 0; (vi<3) && (M_->TV_[t][vi] != v); vi++) { }
      if (vi>=3) return *this; /* Error! This should never happen! */
      vi_ = vi;
      edir_ = -edir_;
      t_ = t;
    } else {
      int vi = (vi_+(3-edir_))%3;
      int t = M_->TT_[t_][vi];
      if (t<0) return *this;
      vi_ = (M_->TTi_[t_][vi]+(3-edir_))%3;
      edir_ = -edir_;
      t_ = t;
    }
    return *this;
  }

  Dart& Dart::orbit0()
  {
    int t = t_;
    alpha1();
    alpha2();
    if (t == t_) alpha1(); /* Undo; boundary. */
    return *this;
  }

  Dart& Dart::orbit1()
  {
    int t = t_;
    alpha2();
    if (t != t_) alpha0(); /* Do only if not at boundary. */
    return *this;
  }

  Dart& Dart::orbit2()
  {
    /* "alpha0(); alpha1();" would be less efficient. */
    vi_ = (vi_+(3+edir_))%3;
    return *this;
  }

  Dart& Dart::orbit0rev()
  {
    int t = t_;
    alpha2();
    if (t != t_) alpha1(); /* Do only if not at boundary. */
    return *this;
  }

  Dart& Dart::orbit1rev() /* Equivalent to orbit1() */
  {
    orbit1();
    return *this;
  }

  Dart& Dart::orbit2rev()
  {
    /* "alpha1(); alpha0();" would be less efficient. */
    vi_ = (vi_+(3-edir_))%3;
    return *this;
  }


  std::ostream& operator<<(std::ostream& output, const Mesh& M)
  {
    //    output << "S =\n" << M.SO();
    output << "TV =\n" << M.TVO();
    output << "TT =\n" << M.TTO();
    if (M.useVT())
      output << "VT =\n" << M.VTO();
    if (M.useTTi())
      output << "TTi =\n" << M.TTiO();
    return output;
  }

  std::ostream& operator<<(std::ostream& output, const MOAint& MO)
  {
    if (!MO.M_) return output;
    for (int i = 0; i < (int)MO.n_; i++) {
      output << ' ' << std::right << std::setw(4)
	     << MO.M_[i];
    }
    std::cout << std::endl;
    return output;
  }

  std::ostream& operator<<(std::ostream& output, const MOAint3& MO)
  {
    if (!MO.M_) return output;
    for (int j = 0; j<3; j++) {
      for (int i = 0; i < (int)MO.n_; i++) {
	output << ' ' << std::right << std::setw(4)
	       << MO.M_[i][j];
      }
      std::cout << std::endl;
    }
    return output;
  }

  std::ostream& operator<<(std::ostream& output, const MOAdouble3& MO)
  {
    if (!MO.M_) return output;
    for (int i = 0; i < (int)MO.n_; i++) {
      for (int j = 0; j<3; j++)
	output << ' ' << std::right << std::setw(10) << std::scientific
	       << MO.M_[i][j];
      std::cout << std::endl;
    }
    return output;
  }

  std::ostream& operator<<(std::ostream& output, const Dart& d)
  {
    output << std::right << std::setw(1) << d.t_
	   << std::right << std::setw(3) << d.edir_
	   << std::right << std::setw(2) << d.vi_;
    if ((!d.isnull()) && (d.t_<(int)d.M()->nT())) {
      output << " ("
	     << d.M()->TV()[d.t_][d.vi_]
	     << ","
	     << d.M()->TV()[d.t_][(d.vi_+(3+d.edir_))%3]
	     << ")";
    }
      
    return output;
  }


} /* namespace fmesh */
