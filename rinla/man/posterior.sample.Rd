%%
%% WARNING! DO NOT EDIT!
%% This file is automatically generated from posterior.sample.R
%%
 \name{inla.sample}
 \alias{inla.posterior.sample}
 \alias{posterior.sample}
 
 \title{Generate samples from an approximated posterior of a fitted model}
 
 \description{This function generate samples from an approximated posterior of a fitted model,  ie an inla-object}
 \usage{
     inla.posterior.sample(n = 1L, result, hyper.user.scale = TRUE, use.improved.mean = TRUE)
 }
 
 \arguments{
   \item{n}{Number of samples.}
   \item{result}{The inla-object, ie the output from an \code{inla}-call. The \code{inla}-object must be created with
                 \code{control.compute=list(config=TRUE)}.}
   \item{hyper.user.scale}{Logical. If \code{TRUE} then values of
   the hyperparameters are given in the user scale (for example
   \code{precision}). If \code{FALSE} then values of the
   hyperparameters are given in the internal representation (for
   example \code{log(precision)}).}
   \item{use.improved.mean}{Logical. If \code{TRUE} then use the
   marginal mean values when constructing samples. If \code{FALSE}
   then use the mean in the Gaussian approximations.}
}
\value{ A list of the samples, where each sample is a list with
  names \code{hyperpar} and \code{latent}, and with their marginal
  densities in \code{logdens$hyperpar} and \code{logdens$latent}
  and the joint density is in \code{logdens$joint}.  THIS IS AN
  EXPERIMENTAL FUNCTION AND CHANGES MAY APPEAR AT ANY TIME!  }
\author{Havard Rue \email{hrue@math.ntnu.no}}
 
\examples{
  r = inla(y ~ 1 ,data = data.frame(y=rnorm(1)), control.compute = list(config=TRUE))
  samples = inla.posterior.sample(2,r)
}
