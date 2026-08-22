module Pinmap where

import System.IO (writeFile)
import System.Directory (doesDirectoryExist,doesFileExist, getCurrentDirectory,createDirectoryIfMissing)
import System.FilePath (takeDirectory, (</>))
import Text.Read (readMaybe)
import Data.List (intercalate)
import Data.Maybe (mapMaybe)

-- d3 : (x,y);analog

data PinType = Analog | Digital | Power | Communication | Special
  deriving Show

pinTypeToJSON :: PinType -> String
pinTypeToJSON Analog        = "analog"
pinTypeToJSON Digital       = "digital"
pinTypeToJSON Power         = "power"
pinTypeToJSON Communication = "communication"
pinTypeToJSON Special       = "special"

data Pin = Pin
  { name    :: String
  , pinType :: PinType
  , x       :: Int
  , y       :: Int
  } deriving Show


splitLine :: String->String->[String]
splitLine elim= go ""
  where
    go acc [] = [reverse acc]
    go acc (c:cs)
      | c `elem` elim = reverse acc:go "" cs
      | otherwise     = go (c:acc) cs


parsePin :: String -> Maybe Pin
parsePin s = case filter (not . null) (splitLine ":,();" (filter (/=' ') s)) of
             [name,xStr,yStr,typeStr] -> do
                px <- readMaybe xStr
                py <- readMaybe yStr
                pinType <- case typeStr of
                  "analog"        -> Just Analog
                  "digital"       -> Just Digital
                  "power"         -> Just Power
                  "communication" -> Just Communication
                  "special"       -> Just Special
                  _        -> Nothing
                return (Pin name pinType px py)
             _                -> Nothing

parseScale :: String -> Maybe (String,Int)
parseScale s = case splitLine "=" (filter (/=' ') s) of
               [scale,value] -> do 
                  x <- readMaybe value
                  return (scale,x)
               _ -> Nothing


findProjectRoot :: FilePath -> IO FilePath
findProjectRoot dir = do
    exists <- doesDirectoryExist (dir </> "GUI")
    if exists
        then return dir
        else let parent = takeDirectory dir
             in if parent == dir
                then error "Projekt-Root (Ordner mit 'GUI') nicht gefunden"
                else findProjectRoot parent


configPath :: String
configPath = "./pins.txt"

outputDir :: String
outputDir = "GUI/gui/data"

pinScale :: String->Int->String
pinScale scale value = "\""++scale++"\":"++ show value++","

pinToJSON::Pin->String
pinToJSON p = "{\"name\":\"" ++ name p ++ "\",\"type\":\"" ++ pinTypeToJSON(pinType p) ++ "\",\"x\":" ++ show (x p) ++ ",\"y\":" ++ show (y p) ++ "}"

pinsToJSON::[Pin]->String
pinsToJSON ps = "\"pins\":[\n" ++ intercalate ",\n" (map pinToJSON ps) ++ "\n]"

main :: IO ()
main = do
    cwd  <- getCurrentDirectory
    root <- findProjectRoot cwd
    let pindataDir = root </> "pindata"
    content <- readFile (pindataDir </> "pins.txt")
    let (scaleOne:scaleTwo:pinLines) = lines content
        pins                         = mapMaybe parsePin pinLines
    createDirectoryIfMissing True pindataDir
    writeFile (pindataDir </> "pins.json") ("{"++ maybe "" (uncurry pinScale) (parseScale scaleOne) 
                                               ++ maybe "" (uncurry pinScale) (parseScale scaleTwo)
                                               ++ pinsToJSON pins++"}")  