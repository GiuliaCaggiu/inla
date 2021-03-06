%%
%% WARNING! DO NOT EDIT!
%% This file is automatically generated from sm.R
%%
\name{inla.as.sparse}
\alias{inla.as.sparse}
\alias{inla.as.dgTMatrix}

\title{Convert a matrix or sparse matrix into the sparse formate used by INLA}

\description{Convert a matrix or sparse matrix into the sparse format used by INLA (dgTMatrix)}

\usage{
 inla.as.sparse(...)
 inla.as.dgTMatrix(A, unique = TRUE)
}

\arguments{
  \item{...}{The arguments. The matrix or sparse matrix,  and the additonal arguments}
  \item{A}{The matrix}
  \item{unique}{If the internal representation needs to be unique or can have duplicated entries.
                Do not use this option unless you know what you are doing.}
}

\value{%%
  \code{inla.as.sparse} and \code{inla.as.dgTMatrix} is the same function.
  The returned value is a sparse matrix in the sparse-format used by INLA
}
%%

\author{Havard Rue \email{hrue@math.ntnu.no}}

\examples{
 A = matrix(1:9, 3, 3)
 inla.as.sparse(A)
}
