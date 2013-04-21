MRuby::Gem::Specification.new('mruby-sdl') do |spec|
  spec.license = 'MIT'
  spec.authors = 'Stephen Belanger <admin@stephenbelanger.com>'
  spec.linker.libraries << ['SDLmain', 'SDL']

  if RUBY_PLATFORM.include?('darwin')
	  spec.cc.flags << '-framework Cocoa'
	end
end