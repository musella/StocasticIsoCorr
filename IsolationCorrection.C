#ifndef __IsolationCorrection__
#define __IsolationCorrection__

#include "TFile.h"
#include "TVectorD.h"
#include "TH1.h"
#include "TRandom.h"
    
#include <iostream>
    
std::vector<float> convertToStd(TVectorD * input)
{
    std::vector<float> ret(input->GetNrows());
    for(int iel=0; iel < input->GetNrows(); ++iel) { ret[iel] = (*input)[iel]; }
    
    return ret;
}
    
class IsolationCorrection
{
    public:
    IsolationCorrection(const char * fname) {
        auto * fin = TFile::Open(fname);

        eta_centers_ = convertToStd(dynamic_cast<TVectorD*>(fin->Get("eta_centers"))); 
        rho_centers_eb_ = convertToStd(dynamic_cast<TVectorD*>(fin->Get("rho_centers_eb"))); 
        rho_centers_ee_ = convertToStd(dynamic_cast<TVectorD*>(fin->Get("rho_centers_ee")));    
        extra_multiplicity_ = convertToStd(dynamic_cast<TVectorD*>(fin->Get("extra_multiplicity")));    

        n_rho_centers_ = std::max(rho_centers_eb_.size(),rho_centers_ee_.size());
        
        histograms_.resize(n_rho_centers_*eta_centers_.size(),0);
        
        for(size_t ieta=0; ieta<eta_centers_.size(); ++ieta) {
            auto & eta = eta_centers_[ieta];
            auto & rho_centers = ( eta<1.5 ? rho_centers_eb_ : rho_centers_ee_ );
            
            for(size_t irho=0; irho<rho_centers.size(); ++irho) {
                auto ibin = index(ieta,irho);
                auto & rho = rho_centers[irho];
                
                TString name = TString::Format("hist_%1.3f_%1.2f",eta,rho);
                name.ReplaceAll(".","p");
                
                auto hist = dynamic_cast<TH1*>(fin->Get(name)->Clone());
                hist->SetDirectory(0);
                
                histograms_[ibin] = hist;
            }
        } 
        
        fin->Close();
    }
    
    float getExtra(float eta, float rho) {
        
        int ieta = findIndex(eta,eta_centers_);
        
        float extraMult = extra_multiplicity_[ieta];
        float floor = std::floor(extraMult);
        auto  ngen = int(floor);
        if( gRandom->Uniform() > extraMult - floor ) { ngen += 1; }
        
        if( ngen == 0 ) { return 0.; }
        
        auto & rho_centers = eta_centers_[ieta] < 1.5 ? rho_centers_eb_ : rho_centers_ee_;
        int irho = findIndex(rho, rho_centers);
        
        auto ibin = index(ieta,irho);        
        auto * hist = histograms_[ibin];
        
        float extra = 0.;
        float thr = hist->GetXaxis()->GetBinLowEdge(2);
        while( ngen > 0 ) {
            --ngen;
            float gen = hist->GetRandom();
            if( gen > thr ) extra += gen;
        }

        return extra;
    }
    
    //private:    
    int index(int ieta, int irho) {
        return ieta*n_rho_centers_ + irho;
    }
        
    int findIndex(float val, std::vector<float> & vec) {
        
        
        return (val < vec.front() ? 0 : (val >= vec.back() ? vec.size() - 1 : 
                                         std::distance(vec.begin(), std::lower_bound(vec.begin(),vec.end(), val) ) ) );
        
    }
    std::vector<TH1 *> histograms_; 
    size_t n_rho_centers_;

    std::vector<float>  eta_centers_, rho_centers_eb_, rho_centers_ee_, extra_multiplicity_;
    
};

#endif // __IsolationCorrection__
