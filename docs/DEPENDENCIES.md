# Distribution and dependency review

JUCE 8.0.6 is pinned by Git commit. It is an external dependency and is not owned by D2NZ. Review JUCE's applicable commercial/AGPL licensing terms at https://juce.com/legal/ before distributing a proprietary binary. No commercial JUCE entitlement is assumed.

Review the bundled VST3 SDK and other third-party notices from the pinned JUCE checkout before shipping. macOS AU requires Apple's SDK. Xcode, code signing and notarization run on a Mac; Apple account requirements are a release prerequisite, not an implemented feature.

D2NZ source files contain original prototype processing, not source from the named commercial reference plugins. No pitch-engine licensing agreement is in place. No legal ownership transfer agreement is implied by these files; formal agreements with any contractors remain necessary.

No blanket open-source licence is assigned to this proprietary project. Before inviting outside contributors, establish contribution and intellectual-property terms.
