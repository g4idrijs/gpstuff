function opt = gp_iaopt(opt, method);
%GP_MCOPT     Default options for GP_INA
%
%             Description
%             OPT = GP_INAOPT(OPT) sets default options for integrated nested
%             approximation (INA) in the structure OPT. The default integration 
%             method is the quasi Monte Carlo with importance samping:
%
%    
%             OPT = GP_INAOPT(OPT, METHOD) sets default options for integrated 
%             nested approximation (INA) with user specified method in the 
%             structure OPT. METHOD is a string defining the integration method.
%             Possibilities and their parameters are:
%   
%             'grid'  = a grid based integration
%                  opt.int_method = 'grid';
%                  opt.stepsize   = 1;
%                  opt.threshold  = 2.5;
%                  opt.validate   = 1;    (check that the integration results are valid)
%
%              'is_normal' = an importance sampling with normal proposal distribution
%                  opt.int_method = 'normal';
%                  opt.nsamples   = 40;
%                  opt.validate   = 1;    (check that the integration results are valid)
%             
%              'is_normal_qmc' = an importance sampling with normal proposal distribution
%                              with quasi Monte Carlo sampling
%                  opt.int_method = 'quasi_mc';
%                  opt.nsamples   = 40;
%                  opt.validate   = 1;    (check that the integration results are valid)
%
%              'is_student-t'     = an importance sampling with Student-t proposal distribution
%                  opt.int_method = 'is_student-t';
%                  opt.nsamples   = 40;
%                  opt.nu         = 4; the degrees of freedom
%                  opt.validate   = 1;    (check that the integration results are valid)
%
%              'mcmc_hmc'      = Markov chain Monte Carlo sampling using Hybrid Monte Carlo
%                  opt.int_method = 'mcmc_hmc';
%                  opt.nsamples = 40;
%                  opt.repeat = 1;
%                  opt.display = 1;
%                  opt.validate   = 1;    (check that the integration results are valid)
%
%                  opt.hmc_opt.steps = 3;
%                  opt.hmc_opt.stepadj = 0.01;
%                  opt.hmc_opt.nsamples = 1;
%                  opt.hmc_opt.persistence = 0;
%                  opt.hmc_opt.persistence_reset = 0;
%                  opt.hmc_opt.decay = 0.8;
%
%             For reference on the method see ...

% Copyright (c) 2009 Jarno Vanhatalo

% This software is distributed under the GNU General Public 
% License (version 2 or later); please refer to the file 
% License.txt, included with the software, for details.

if nargin < 1
    opt=[];
end

opt.fminunc=optimset('GradObj','on');
opt.fminunc=optimset(opt.fminunc,'LargeScale', 'on');
opt.fminunc=optimset(opt.fminunc,'Display', 'off');

if nargin < 2
    method = 'is_normal_qmc';
end

switch method
  case 'grid'
    opt.int_method = 'grid';
    opt.stepsize = 1;
    opt.threshold = 2.5;
    opt.validate = 1;
  case 'is_normal'
    opt.int_method = 'is_normal';
    opt.nsamples = 40;
    opt.validate = 1;
    opt.qmc = 0;
    opt.improved = 0;
  case 'is_normal_qmc'
    opt.int_method = 'is_normal';
    opt.nsamples = 40;
    opt.validate = 1;
    opt.qmc = 1;
    opt.improved = 0;
  case 'is_student-t'
    opt.int_method = 'is_student-t';
    opt.nsamples = 40;
    opt.nu = 4;
    opt.validate = 1;
    opt.improved = 0;
  case 'mcmc_hmc'
    opt.int_method = 'mcmc_hmc';
    opt.nsamples = 40;
    opt.repeat = 1;
    opt.display = 1;
    opt.validate = 1;
    
    % Set the hmc sampling options
    opt.hmc_opt.steps = 3;
    opt.hmc_opt.stepadj = 0.01;
    opt.hmc_opt.nsamples = 1;
    opt.hmc_opt.persistence = 0;
    opt.hmc_opt.persistence_reset = 0;
    opt.hmc_opt.decay = 0.8;
  case 'mcmc_sls'
    opt.int_method = 'mcmc_sls';
    opt.nsamples = 40;
    opt.repeat = 1;
    opt.display = 1;
    
    % Set the sls sampling options
  case 'CCD'
    opt.int_method = 'CCD';
    opt.improved = 0;
    opt.stepsize = 1;
    opt.f0 = 1.1;
end    