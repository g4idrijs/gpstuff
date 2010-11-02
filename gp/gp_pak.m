function w = gp_pak(gp, param)
%GP_PAK	 Combine GP hyper-parameters into one vector.
%
%	Description
%        W = GP_PAK(GP, PARAM) takes a Gaussian Process data structure
%        GP and string PARAM defining, which parameters are packed and
%        combines the parameters into a single row vector W. If PARAM
%        is not given the function packs all parameters.
%
%        Each of the following strings in PARAM defines one group of
%        parameters to pack:
%         'covariance'     = pack hyperparameters of covariance
%                            function
%         'likelihood'     = pack parameters of likelihood
%         'inducing'       = pack inducing inputs (in sparse
%                            approximations): W = gp.X_u(:)
%
%        By combining the strings one can pack more than one group of
%        parameters. For example:
%         'covariance+inducing' = pack covariance function parameters
%                                 and inducing inputs
%         'covariance+likelih'  = pack covariance function parameters
%                                 of likelihood parameters
%
%        Inside each group (such as covariance functions) the
%        parameters to be packed is defined by the existence of a
%        prior structure. For example, if GP has two covariance
%        functions but only the first one has prior for its parameters
%        then only the parameters of the first one are packed. Thus,
%        also inducing inputs require prior if they are to be
%        optimized.
%
%        gp_pak and gp_unpak functions are used when GP parameters are
%        optimized or sampled with gp_mc. The same PARAM string should
%        be given for all of these functions.
%
%        See also
%        GP_UNPAK
%

% Copyright (c) 2007-2010 Jarno Vanhatalo

% This software is distributed under the GNU General Public
% License (version 2 or later); please refer to the file
% License.txt, included with the software, for details.

    w = [];

    if isfield(gp,'etr') && length(gp.etr) > 1
        if strcmp(gp.type, 'PIC_BLOCK') || strcmp(gp.type, 'PIC')
            ind = gp.tr_index;           % block indeces for training points
            gp = rmfield(gp,'tr_index');
        end
        ns = length(gp.etr);
        for i1 = 1:ns
            Gp = take_nth(gp,i1);
            w(i1,:) = gp_pak(Gp);
        end
    else
        
        if nargin < 2
            param = gp.infer_params;
        end
        
        % Pack the hyperparameters of covariance functions
        if ~isempty(strfind(param, 'covariance'))
            ncf = length(gp.cf);
            
            for i=1:ncf
                gpcf = gp.cf{i};
                w = [w feval(gpcf.fh.pak, gpcf)];
            end
            
            if isfield(gp, 'noisef')
                nn = length(gp.noisef);
                for i=1:nn
                    noisef = gp.noisef{i};
                    w = [w feval(noisef.fh.pak, noisef)];
                end
            end
        end
        
        % Pack the inducing inputs
        if ~isempty(strfind(param, 'inducing'))
            if isfield(gp,'p') && isfield(gp.p, 'X_u') && ~isempty(gp.p.X_u)
                w = [w gp.X_u(:)'];
            end
        end
        
        % Pack the hyperparameters of likelihood function
        if ~isempty(strfind(param, 'likelihood'))
            if isstruct(gp.lik)
                w = [w feval(gp.lik.fh.pak, gp.lik)];
            end
        end
        
    end

end